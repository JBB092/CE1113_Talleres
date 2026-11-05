/*
 * riscv_edu_matmul.c — Acelerador educativo de multiplicación de matrices 4×4
 *
 * Dispositivo MMIO para la máquina "virt" de QEMU (RISC-V 32-bit).
 * Tipo QEMU : "riscv-edu-matmul"
 * Base MMIO : 0x10011000
 *
 * ===========================================================================
 * MAPA DE REGISTROS (todos de 32 bits, acceso word alineado)
 *
 *  Offset      Nombre       Acceso  Descripción
 *  0x00–0x3C   A[0..15]     W       Matriz A — elemento [i][j] en 0x00+(i*4+j)*4
 *  0x40–0x7C   B[0..15]     W       Matriz B — mismo layout
 *  0x80–0xBC   C[0..15]     R       Resultado C = A×B  (uint32 por elemento)
 *  0xC0        CONTROL      W       bit 0: START — dispara el cómputo
 *  0xC4        STATUS       R       bit 0: DONE,  bit 1: BUSY
 *
 * Protocolo de uso desde firmware:
 *   1. Escribir los 16 elementos de A (offsets 0x00–0x3C).
 *   2. Escribir los 16 elementos de B (offsets 0x40–0x7C).
 *   3. Escribir CONTROL = 0x1 (START).
 *   4. Leer STATUS hasta que bit DONE (bit 0) esté activo.
 *   5. Leer los 16 elementos del resultado C (offsets 0x80–0xBC).
 * ===========================================================================
 *
 * TALLER 07 — Tareas del estudiante
 * ==================================
 * Este archivo compila y arranca sin errores, pero el cómputo no está
 * implementado.  Debe completar exactamente tres secciones marcadas TODO:
 *
 *   TODO 1 — matmul_compute() : calcular C = A × B.
 *   TODO 2 — write handler   : disparar el cómputo y actualizar STATUS.
 *   TODO 3 — read handler    : exponer STATUS y los elementos de C.
 *
 * NO modifique las secciones marcadas "NO MODIFICAR":
 *   instance_init, reset, VMState, class_init, TypeInfo, type_init.
 */

/* ----------------------------------------------------------------------------
 * Cabeceras estándar de QEMU
 *
 * qemu/osdep.h      — PRIMER include obligatorio en todo archivo QEMU;
 *                     configura tipos portables y macros del sistema.
 * hw/core/sysbus.h  — SysBusDevice, sysbus_init_mmio(), sysbus_realize_and_unref()
 * migration/vmstate.h — macros VMSTATE_* para serialización de estado.
 * qom/object.h      — sistema de objetos: OBJECT_DECLARE_SIMPLE_TYPE, TypeInfo.
 * qemu/module.h     — type_init().
 * --------------------------------------------------------------------------*/
#include "qemu/osdep.h"
#include "hw/core/sysbus.h"
#include "migration/vmstate.h"
#include "qom/object.h"
#include "qemu/module.h"
#include "qemu/timer.h"

/* ----------------------------------------------------------------------------
 * Identificador de tipo y macro de acceso seguro
 *
 * TYPE_RISCV_EDU_MATMUL es la cadena que identifica al dispositivo en el
 * sistema de tipos de QEMU.  Es la misma cadena usada en virt.c:
 *   qdev_new("riscv-edu-matmul")
 *
 * OBJECT_DECLARE_SIMPLE_TYPE genera automáticamente:
 *   - La declaración forward de RISCVEduMatmulState.
 *   - El macro de cast seguro RISCV_EDU_MATMUL(obj) que verifica el tipo
 *     en modo debug y hace el cast a RISCVEduMatmulState*.
 * --------------------------------------------------------------------------*/
#define TYPE_RISCV_EDU_MATMUL "riscv-edu-matmul"
OBJECT_DECLARE_SIMPLE_TYPE(RISCVEduMatmulState, RISCV_EDU_MATMUL)

/* ----------------------------------------------------------------------------
 * Offsets de los registros dentro de la región MMIO
 *
 * El parámetro `addr` en los callbacks read/write es el offset desde la base
 * de la región (0x10011000), NO la dirección física absoluta.
 * Ejemplo: acceso a 0x10011080 → addr = 0x80 (inicio de la región C).
 * --------------------------------------------------------------------------*/
#define REG_A_BASE    0x00u   /* A[0..15]: offsets 0x00, 0x04, ... 0x3C */
#define REG_B_BASE    0x40u   /* B[0..15]: offsets 0x40, 0x44, ... 0x7C */
#define REG_C_BASE    0x80u   /* C[0..15]: offsets 0x80, 0x84, ... 0xBC (read-only) */
#define REG_CONTROL   0xC0u
#define REG_STATUS    0xC4u
#define MATMUL_MMIO_SIZE 0x1000u   /* tamaño de la región MMIO: 4 KiB */

/* Bits de control y estado */
#define CTRL_START    (1u << 0)   /* W: el firmware escribe 1 para disparar cómputo */
#define STATUS_DONE   (1u << 0)   /* R: el device activa 1 cuando el resultado está listo */
#define STATUS_BUSY   (1u << 1)   /* R: el device activa 1 mientras procesa */

#define MAT_N 4u   /* dimensión N de la matriz cuadrada N×N */
#define MATMUL_LATENCY_US 1000u

/* ----------------------------------------------------------------------------
 * Estructura de estado del dispositivo  (equivalente a los flip-flops en RTL)
 *
 * Contiene todos los "registros" del dispositivo.  QEMU pasa un puntero a
 * esta estructura como el parámetro `opaque` en los callbacks read/write.
 *
 * REGLA OBLIGATORIA: el primer campo SIEMPRE debe ser el tipo padre
 * (SysBusDevice).  El sistema de objetos de QEMU depende de que la
 * estructura comience con el padre para poder hacer casts entre tipos.
 * --------------------------------------------------------------------------*/
typedef struct RISCVEduMatmulState {
    SysBusDevice parent_obj;   /* herencia — DEBE ser el primer campo */
    MemoryRegion mmio;         /* descriptor de la región MMIO */

    /* Registros del periférico */
    uint32_t a[MAT_N * MAT_N];   /* operando A — uint8 en bits[7:0] de cada word */
    uint32_t b[MAT_N * MAT_N];   /* operando B — ídem */
    uint32_t c[MAT_N * MAT_N];   /* resultado C = A×B — uint32 por elemento */
    uint32_t status;             /* registro de estado: bits DONE / BUSY */
    QEMUTimer *latency_timer;
} RISCVEduMatmulState;

/* ============================================================================
 * TODO 1 — Implementar la multiplicación de matrices  C = A × B
 *
 * Esta función es el "núcleo aritmético" del acelerador.  Calcula el producto
 * de las matrices s->a y s->b y almacena el resultado en s->c.
 *
 * Índice lineal: el elemento [i][j] de una matriz 4×4 está en el índice
 * lineal  i * MAT_N + j  del array correspondiente.
 *
 * Tipos de datos:
 *   - s->a[k] y s->b[k] son uint32_t; el valor útil está en los bits [7:0].
 *     (El firmware escribe uint8 en un registro de 32 bits.)
 *   - s->c[k] debe ser uint32_t.  El valor máximo de C[i][j] es:
 *     4 × 255 × 255 = 260 100, que cabe en 19 bits.
 *
 * Algoritmo (triple loop estándar de multiplicación de matrices):
 *
 *   for i = 0 .. MAT_N-1:
 *     for j = 0 .. MAT_N-1:
 *       c[i*MAT_N + j] = 0
 *       for k = 0 .. MAT_N-1:
 *         c[i*MAT_N + j] += a[i*MAT_N + k] * b[k*MAT_N + j]
 * ========================================================================= */
static void __attribute__((unused)) matmul_compute(RISCVEduMatmulState *s)
{
    for (uint32_t i = 0; i < MAT_N; i++) {
        for (uint32_t j = 0; j < MAT_N; j++) {
            s->c[i * MAT_N + j] = 0;
            for (uint32_t k = 0; k < MAT_N; k++) {
                s->c[i * MAT_N + j] += (s->a[i * MAT_N + k] & 0xFFu) *
                                       (s->b[k * MAT_N + j] & 0xFFu);
            }
        }
    }
}

static void matmul_latency_cb(void *opaque)
{
    RISCVEduMatmulState *s = opaque;
    matmul_compute(s);
    s->status = STATUS_DONE;
}

/* ============================================================================
 * Handler de lectura MMIO
 *
 * QEMU invoca esta función cada vez que el CPU ejecuta una instrucción `lw`
 * cuya dirección cae dentro de la región MMIO de este dispositivo.
 *
 * Parámetros:
 *   opaque — puntero al RISCVEduMatmulState (pasado en memory_region_init_io)
 *   addr   — offset dentro de la región MMIO (0x00 ... 0xFFF)
 *   size   — ancho del acceso en bytes (siempre 4 dado valid.min/max = 4)
 *
 * Retorno: uint64_t — QEMU usa 64 bits internamente; el firmware recibirá
 *          solo los 32 bits bajos en el registro de destino de `lw`.
 * ========================================================================= */
static uint64_t riscv_edu_matmul_read(void *opaque, hwaddr addr, unsigned size)
{
    RISCVEduMatmulState *s = opaque;
    (void)size;

    /* A y B son write-only: lectura siempre devuelve 0 */
    if (addr < REG_B_BASE) {
        return 0;
    }
    if (addr >= REG_B_BASE && addr < REG_C_BASE) {
        return 0;
    }

    /* TODO 3a — Devolver el elemento correcto de s->c[].
     *
     * La región C ocupa offsets 0x80 a 0xBC (16 elementos × 4 bytes).
     * El índice lineal del elemento se obtiene como:
     *   idx = (addr - REG_C_BASE) / 4
     * Verificar que idx < MAT_N * MAT_N antes de acceder al array.
     */
    if (addr >= REG_C_BASE && addr < REG_CONTROL) {
        uint32_t idx = (addr - REG_C_BASE) / 4u;
        if (idx < MAT_N * MAT_N) {
            return s->c[idx];
        }
        return 0;
    }

    switch (addr) {
    case REG_CONTROL:
        return 0;   /* write-only */

    case REG_STATUS:
        /* TODO 3b — Devolver s->status.
         *
         * El firmware sondea este registro en un bucle hasta leer
         * STATUS_DONE.  Si esta función siempre devuelve 0, el firmware
         * quedará bloqueado esperando un DONE que nunca llega.
         */
        return s->status;

    default:
        return 0;
    }
}

/* ============================================================================
 * Handler de escritura MMIO
 *
 * QEMU invoca esta función cada vez que el CPU ejecuta una instrucción `sw`
 * cuya dirección cae dentro de la región MMIO de este dispositivo.
 *
 * Parámetros:
 *   opaque — puntero al RISCVEduMatmulState
 *   addr   — offset dentro de la región MMIO
 *   value  — valor de 64 bits escrito por el CPU (los 32 bits altos son 0)
 *   size   — ancho del acceso en bytes
 * ========================================================================= */
static void riscv_edu_matmul_write(void *opaque, hwaddr addr, uint64_t value,
                                   unsigned size)
{
    RISCVEduMatmulState *s = opaque;
    /* Truncar a 8 bits: los operandos son uint8_t */
    uint32_t v = (uint32_t)(value & 0xFFu);
    (void)size;

    /* --- Operandos A y B: almacenamiento directo, sin lógica adicional --- */
    if (addr < REG_B_BASE) {
        uint32_t idx = (addr - REG_A_BASE) / 4u;
        if (idx < MAT_N * MAT_N) {
            s->a[idx] = v;
        }
        return;
    }

    if (addr >= REG_B_BASE && addr < REG_C_BASE) {
        uint32_t idx = (addr - REG_B_BASE) / 4u;
        if (idx < MAT_N * MAT_N) {
            s->b[idx] = v;
        }
        return;
    }

    /* C es read-only: ignorar silenciosamente cualquier escritura */
    if (addr >= REG_C_BASE && addr < REG_CONTROL) {
        return;
    }

    switch (addr) {
    case REG_CONTROL:
        /* TODO 2 — Disparar el cómputo cuando el firmware activa START.
         *
         * Secuencia esperada (modo síncrono):
         *   1. Verificar: if (v & CTRL_START)
         *   2. Indicar que el device está procesando: s->status = STATUS_BUSY
         *   3. Ejecutar el cómputo: matmul_compute(s)
         *   4. Indicar que el resultado está listo: s->status = STATUS_DONE
         *
         * En el modo síncrono los cuatro pasos ocurren dentro de este
         * callback antes de retornar.  El firmware verá STATUS_DONE
         * activo en su primera lectura de STATUS después del `sw`.
         */
        if (v & CTRL_START) {
            s->status = STATUS_BUSY;
            timer_mod(s->latency_timer,
                      qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) +
                      (int64_t)MATMUL_LATENCY_US * 1000);
        }
        break;

    default:
        break;
    }
}

/* ============================================================================
 * MemoryRegionOps — tabla que conecta los callbacks con la región MMIO
 *
 * endianness    : DEVICE_LITTLE_ENDIAN — igual que RISC-V en bare-metal.
 * valid.min/max : solo accesos de 4 bytes (word).  QEMU rechaza automáti-
 *                 camente accesos de 1 o 2 bytes; no es necesario validar
 *                 en los callbacks.
 *
 * NO MODIFICAR
 * ========================================================================= */
static const MemoryRegionOps riscv_edu_matmul_mmio_ops = {
    .read  = riscv_edu_matmul_read,
    .write = riscv_edu_matmul_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

/* ============================================================================
 * reset — llevar todos los registros al estado de encendido (power-on reset)
 *
 * NO MODIFICAR
 * ========================================================================= */
static void riscv_edu_matmul_reset(DeviceState *dev)
{
    RISCVEduMatmulState *s = RISCV_EDU_MATMUL(dev);
    timer_del(s->latency_timer);
    memset(s->a, 0, sizeof(s->a));
    memset(s->b, 0, sizeof(s->b));
    memset(s->c, 0, sizeof(s->c));
    s->status = 0;
}

/* ============================================================================
 * instance_init — constructor de instancia
 *
 * NO MODIFICAR
 * ========================================================================= */
static void riscv_edu_matmul_init(Object *obj)
{
    RISCVEduMatmulState *s   = RISCV_EDU_MATMUL(obj);
    SysBusDevice        *sbd = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->mmio, obj, &riscv_edu_matmul_mmio_ops, s,
                          TYPE_RISCV_EDU_MATMUL, MATMUL_MMIO_SIZE);
    sysbus_init_mmio(sbd, &s->mmio);
    s->latency_timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, matmul_latency_cb, s);
}

/* ============================================================================
 * VMStateDescription — tabla para migración y snapshots de estado
 *
 * NO MODIFICAR
 * ========================================================================= */
static const VMStateDescription vmstate_riscv_edu_matmul = {
    .name = TYPE_RISCV_EDU_MATMUL,
    .version_id = 1,
    .minimum_version_id = 1,
    .fields = (const VMStateField[]) {
        VMSTATE_UINT32_ARRAY(a,      RISCVEduMatmulState, MAT_N * MAT_N),
        VMSTATE_UINT32_ARRAY(b,      RISCVEduMatmulState, MAT_N * MAT_N),
        VMSTATE_UINT32_ARRAY(c,      RISCVEduMatmulState, MAT_N * MAT_N),
        VMSTATE_UINT32(status,       RISCVEduMatmulState),
        VMSTATE_END_OF_LIST()
    }
};

/* ============================================================================
 * class_init — inicialización de la clase (se ejecuta UNA SOLA VEZ por tipo)
 *
 * NO MODIFICAR
 * ========================================================================= */
static void riscv_edu_matmul_class_init(ObjectClass *klass, const void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->desc = "Educational 4x4 uint8 matrix multiplier";
    dc->vmsd = &vmstate_riscv_edu_matmul;
    device_class_set_legacy_reset(dc, riscv_edu_matmul_reset);
}

/* ============================================================================
 * TypeInfo — descriptor del tipo para el sistema de objetos de QEMU
 *
 * NO MODIFICAR
 * ========================================================================= */
static const TypeInfo riscv_edu_matmul_info = {
    .name          = TYPE_RISCV_EDU_MATMUL,
    .parent        = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(RISCVEduMatmulState),
    .instance_init = riscv_edu_matmul_init,
    .class_init    = riscv_edu_matmul_class_init,
};

/* ============================================================================
 * Registro del tipo — punto de entrada del módulo
 *
 * NO MODIFICAR
 * ========================================================================= */
static void riscv_edu_matmul_register_types(void)
{
    type_register_static(&riscv_edu_matmul_info);
}

type_init(riscv_edu_matmul_register_types);
#include "mbed.h"

static DMA_TypeDef *dma2 = DMA2;
static DMA_Stream_TypeDef *dma2_stream = DMA2_Stream1;

static Serial pc(USBTX, USBRX);

static uint32_t from_value = 0x01010101;
static uint32_t to_value = 0x0;

static void disable_dma(void)
{
	dma2_stream->CR &= ~DMA_SxCR_EN;
	while ((dma2_stream->CR & DMA_SxCR_EN) == DMA_SxCR_EN) {
		wait(0.001);
	}
}

static void enable_dma(void)
{
	uint32_t lifcr_mask = DMA_LIFCR_CFEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTCIF1;
	dma2->LIFCR |= lifcr_mask;
	dma2_stream->CR |= DMA_SxCR_EN;
}

static void setup_transfer(void *to, void *from, uint32_t num)
{
	dma2_stream->CR = DMA_SxCR_DIR_1 |
	                  DMA_SxCR_TCIE |
	                  DMA_SxCR_TEIE |
	                  DMA_SxCR_DMEIE |
	                  DMA_SxCR_PSIZE_1 |
	                  DMA_SxCR_MSIZE_1 |
	                  DMA_SxCR_CHSEL_1 |
	                  DMA_SxCR_CHSEL_2;
	dma2_stream->PAR = (uint32_t)from;
	dma2_stream->M0AR = (uint32_t)to;
	dma2_stream->NDTR = num;
}

static void show_dma_regs(void)
{
	pc.printf("DMA2 LISR: 0x%08x\n", dma2->LISR);
	pc.printf("DMA2 HISR: 0x%08x\n", dma2->HISR);
	pc.printf("DMA2 CR: 0x%08x\n", dma2_stream->CR);
	pc.printf("DMA2 NDTR: 0x%08x\n", dma2_stream->NDTR);
	pc.printf("DMA2 PAR: 0x%08x\n", dma2_stream->PAR);
	pc.printf("DMA2 M0AR: 0x%08x\n", dma2_stream->M0AR);
}

static void dma_irq_handler(void)
{
	static const uint32_t errors = DMA_LISR_FEIF1 | DMA_LISR_DMEIF1 | DMA_LISR_TEIF1;
	uint32_t lisr = dma2->LISR;

	if ((lisr & errors) == errors) {
		pc.printf("DMA transfer error: lisr: 0x%08x\n", lisr);
	} else {
		if ((lisr & DMA_LISR_TCIF1) == DMA_LISR_TCIF1) {
			pc.printf("DMA transfer complete, from: %08x, to: %08x\n", from_value, to_value);
			from_value++;
			enable_dma();
		} else {
			pc.printf("DMA transfer incomplete, lisr: 0x%08x\n", lisr);
		}
	}

	uint32_t lifcr_mask = DMA_LIFCR_CFEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTCIF1;
	dma2->LIFCR |= lifcr_mask;
}

int main(void)
{
	RCC_TypeDef *rcc = RCC;
	rcc->AHB1ENR |= RCC_AHB1ENR_DMA2EN;;

	pc.printf("\n");
	disable_dma();
	setup_transfer(&to_value, &from_value, 1);
	show_dma_regs();

	NVIC_SetVector(DMA2_Stream1_IRQn, (uint32_t)&dma_irq_handler);
	NVIC_EnableIRQ(DMA2_Stream1_IRQn);

	pc.printf("Enable DMA\n");
	enable_dma();
	wait(1.0);
	show_dma_regs();

	while (true) {
	}
}



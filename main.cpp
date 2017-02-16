#include "mbed.h"

static DMA_TypeDef *dma2 = DMA2;
static DMA_Stream_TypeDef *dma2_stream0 = DMA2_Stream0;

static Serial pc(USBTX, USBRX);

static void disable_dma(void)
{
	dma2_stream0->CR &= ~DMA_SxCR_EN;
	while ((dma2_stream0->CR & DMA_SxCR_EN) == DMA_SxCR_EN) {
		wait(0.001);
	}
}

static void enable_dma(void)
{
	uint32_t lifcr_mask = DMA_LIFCR_CFEIF0 | DMA_LIFCR_CDMEIF0 | DMA_LIFCR_CTEIF0 | DMA_LIFCR_CHTIF0 | DMA_LIFCR_CTCIF0;
	dma2->LIFCR |= lifcr_mask;
	dma2_stream0->CR |= DMA_SxCR_EN;
}

static void setup_transfer(void *to, void *from, uint32_t num)
{
	dma2_stream0->CR = DMA_SxCR_DIR_1 | DMA_SxCR_TCIE | DMA_SxCR_TEIE | DMA_SxCR_DMEIE;
	dma2_stream0->PAR = (uint32_t)from;
	dma2_stream0->M0AR = (uint32_t)to;
	dma2_stream0->NDTR = num;
}

static void show_dma_regs(void)
{
	pc.printf("DMA2 LISR: %x\n", dma2->LISR);
	pc.printf("DMA2 HISR: %x\n", dma2->HISR);
	pc.printf("DMA2 CR: %8x\n", dma2_stream0->CR);
	pc.printf("DMA2 NDTR: %8x\n", dma2_stream0->NDTR);
	pc.printf("DMA2 PAR: %8x\n", dma2_stream0->PAR);
	pc.printf("DMA2 M0AR: %8x\n", dma2_stream0->M0AR);
}

int main(void) {
	uint32_t from = 0x1;
	uint32_t to = 0x2;

	RCC_TypeDef *rcc = RCC;
	rcc->AHB1ENR |= RCC_AHB1ENR_DMA2EN;;

	pc.printf("\n");
	disable_dma();
	setup_transfer(&to, &from, sizeof(from));
	show_dma_regs();

	pc.printf("Enable DMA\n");
	enable_dma();
	wait(1.0);
	pc.printf("Looking at vars: %x %x\n", from, to);
	show_dma_regs();

	from = 5;
	to = 6;
	pc.printf("Enable DMA\n");
	enable_dma();
	wait(1.0);
	pc.printf("Looking at vars: %x %x\n", from, to);
	show_dma_regs();

	while (true) {
	}
}



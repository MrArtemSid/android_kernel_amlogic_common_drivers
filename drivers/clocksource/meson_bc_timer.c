// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Amlogic, Inc. All rights reserved.
 *
 * Broadcast clock event device on an EE timer (Timer F on SM1), which keeps
 * running while the cores are powered down in an idle state that stops their
 * local timers.
 */

#include <linux/clockchips.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>

/* The timer counts down 16 bits at the 1 us resolution */
#define MESON_BC_TIMER_RATE		1000000
#define MESON_BC_TIMER_MAX_DELTA	0xfffe
#define MESON_BC_TIMER_RESOLUTION_1US	0
#define MESON_BC_TIMER_RESOLUTION_BITS	2

struct meson_bc_timer {
	struct clock_event_device ced;
	void __iomem *mux_reg;
	void __iomem *reg;
	u32 bit_enable;
	u32 bit_mode;
	u32 bit_resolution;
	/* The mux register holds the controls of the other timers too */
	spinlock_t lock;
};

static inline struct meson_bc_timer *to_meson_bc_timer(struct clock_event_device *ced)
{
	return container_of(ced, struct meson_bc_timer, ced);
}

static void meson_bc_timer_set_bits(void __iomem *reg, u32 value, u32 start, u32 len)
{
	u32 mask = GENMASK(start + len - 1, start);

	writel_relaxed((readl_relaxed(reg) & ~mask) | ((value << start) & mask), reg);
}

static void meson_bc_timer_mux(struct meson_bc_timer *t, u32 value, u32 start, u32 len)
{
	unsigned long flags;

	spin_lock_irqsave(&t->lock, flags);
	meson_bc_timer_set_bits(t->mux_reg, value, start, len);
	spin_unlock_irqrestore(&t->lock, flags);
}

static void meson_bc_timer_load(struct meson_bc_timer *t, unsigned long cycles)
{
	/* Write the count twice to clear the previous trigger cleanly */
	writel_relaxed(readl_relaxed(t->reg) | (cycles & 0xffff), t->reg);
	meson_bc_timer_set_bits(t->reg, cycles, 0, 16);
}

static int meson_bc_timer_set_next_event(unsigned long cycles,
					 struct clock_event_device *ced)
{
	meson_bc_timer_load(to_meson_bc_timer(ced), cycles);
	return 0;
}

static int meson_bc_timer_shutdown(struct clock_event_device *ced)
{
	struct meson_bc_timer *t = to_meson_bc_timer(ced);

	meson_bc_timer_mux(t, 0, t->bit_enable, 1);
	return 0;
}

static int meson_bc_timer_set_periodic(struct clock_event_device *ced)
{
	struct meson_bc_timer *t = to_meson_bc_timer(ced);

	meson_bc_timer_load(t, DIV_ROUND_CLOSEST(MESON_BC_TIMER_RATE, HZ));
	meson_bc_timer_mux(t, 1, t->bit_mode, 1);
	meson_bc_timer_mux(t, 1, t->bit_enable, 1);
	return 0;
}

static int meson_bc_timer_set_oneshot(struct clock_event_device *ced)
{
	struct meson_bc_timer *t = to_meson_bc_timer(ced);

	meson_bc_timer_mux(t, 0, t->bit_mode, 1);
	meson_bc_timer_mux(t, 1, t->bit_enable, 1);
	return 0;
}

static int meson_bc_timer_resume(struct clock_event_device *ced)
{
	struct meson_bc_timer *t = to_meson_bc_timer(ced);

	meson_bc_timer_mux(t, 1, t->bit_enable, 1);
	return 0;
}

static irqreturn_t meson_bc_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *ced = dev_id;

	if (ced->event_handler)
		ced->event_handler(ced);
	return IRQ_HANDLED;
}

static int meson_bc_timer_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct meson_bc_timer *t;
	int irq, ret;

	t = devm_kzalloc(dev, sizeof(*t), GFP_KERNEL);
	if (!t)
		return -ENOMEM;
	spin_lock_init(&t->lock);

	t->mux_reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(t->mux_reg))
		return PTR_ERR(t->mux_reg);
	t->reg = devm_platform_ioremap_resource(pdev, 1);
	if (IS_ERR(t->reg))
		return PTR_ERR(t->reg);

	if (of_property_read_u32(np, "bit_enable", &t->bit_enable) ||
	    of_property_read_u32(np, "bit_mode", &t->bit_mode) ||
	    of_property_read_u32(np, "bit_resolution", &t->bit_resolution) ||
	    of_property_read_u32(np, "clockevent-rating", &t->ced.rating) ||
	    of_property_read_u32(np, "clockevent-features", &t->ced.features)) {
		dev_err(dev, "missing timer properties\n");
		return -EINVAL;
	}
	if (of_property_read_string(np, "timer_name", &t->ced.name))
		t->ced.name = dev_name(dev);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	t->ced.irq = irq;
	t->ced.cpumask = cpu_possible_mask;
	t->ced.set_next_event = meson_bc_timer_set_next_event;
	t->ced.set_state_shutdown = meson_bc_timer_shutdown;
	t->ced.set_state_periodic = meson_bc_timer_set_periodic;
	t->ced.set_state_oneshot = meson_bc_timer_set_oneshot;
	t->ced.tick_resume = meson_bc_timer_resume;

	/* Stopped, one-shot, at the 1 us resolution */
	meson_bc_timer_mux(t, 0, t->bit_enable, 1);
	meson_bc_timer_mux(t, 0, t->bit_mode, 1);
	meson_bc_timer_mux(t, MESON_BC_TIMER_RESOLUTION_1US, t->bit_resolution,
			   MESON_BC_TIMER_RESOLUTION_BITS);

	ret = devm_request_irq(dev, irq, meson_bc_timer_interrupt, IRQF_TIMER,
			       t->ced.name, &t->ced);
	if (ret) {
		dev_err(dev, "failed to request irq %d: %d\n", irq, ret);
		return ret;
	}

	clockevents_config_and_register(&t->ced, MESON_BC_TIMER_RATE, 1,
					MESON_BC_TIMER_MAX_DELTA);
	dev_info(dev, "%s registered as clock event device\n", t->ced.name);
	return 0;
}

static const struct of_device_id meson_bc_timer_of_match[] = {
	{ .compatible = "amlogic,bc-timer" },
	{ .compatible = "arm, meson-bc-timer" },
	{ }
};
MODULE_DEVICE_TABLE(of, meson_bc_timer_of_match);

static struct platform_driver meson_bc_timer_driver = {
	.probe = meson_bc_timer_probe,
	.driver = {
		.name = "meson-bc-timer",
		.of_match_table = meson_bc_timer_of_match,
		/* A clock event device cannot be unregistered */
		.suppress_bind_attrs = true,
	},
};
builtin_platform_driver(meson_bc_timer_driver);

MODULE_DESCRIPTION("Amlogic Meson broadcast timer");
MODULE_LICENSE("GPL");

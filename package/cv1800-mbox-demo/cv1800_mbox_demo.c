// SPDX-License-Identifier: GPL-2.0
/*
 * cv1800_mbox_demo.c — mailbox client for SG2002 inter-core IPC
 *
 * Platform driver binding to compatible "cv1800-mbox-demo".
 * Requests channel 0 via DT mboxes property for self-send
 * loopback (C906B -> C906B). Exposes debugfs for userspace.
 *
 * Usage:
 *   echo "abc12345" > /sys/kernel/debug/cv1800_mbox_demo/send
 *   cat /sys/kernel/debug/cv1800_mbox_demo/recv
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/mailbox_client.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/of.h>

#define CHAN_NAME  "cv1800-mbox-demo"
#define MSG_LEN    8

struct cv1800_mbox_demo {
	struct mbox_client mbox_cl;
	struct mbox_chan *mbox_chan;
	struct dentry *debugfs_dir;
	u8 last_recv[MSG_LEN];
	int recv_count;
};

static void mbox_rx_callback(struct mbox_client *cl, void *data)
{
	struct cv1800_mbox_demo *demo =
		container_of(cl, struct cv1800_mbox_demo, mbox_cl);

	memcpy(demo->last_recv, data, MSG_LEN);
	demo->recv_count++;
}

static ssize_t send_write(struct file *file, const char __user *buf,
			   size_t count, loff_t *ppos)
{
	struct cv1800_mbox_demo *demo = file->f_inode->i_private;
	u8 msg[MSG_LEN] = {0};
	int ret;

	if (count > MSG_LEN)
		count = MSG_LEN;

	if (copy_from_user(msg, buf, count))
		return -EFAULT;

	ret = mbox_send_message(demo->mbox_chan, msg);
	if (ret < 0)
		return ret;

	return count;
}

static ssize_t recv_read(struct file *file, char __user *buf,
			  size_t count, loff_t *ppos)
{
	struct cv1800_mbox_demo *demo = file->f_inode->i_private;
	char tmp[MSG_LEN + 1] = {0};

	memcpy(tmp, demo->last_recv, MSG_LEN);
	return simple_read_from_buffer(buf, count, ppos, tmp, MSG_LEN);
}

static ssize_t count_read(struct file *file, char __user *buf,
			   size_t count, loff_t *ppos)
{
	struct cv1800_mbox_demo *demo = file->f_inode->i_private;
	char tmp[32];
	int len = snprintf(tmp, sizeof(tmp), "%d\n", demo->recv_count);

	return simple_read_from_buffer(buf, count, ppos, tmp, len);
}

static const struct file_operations send_fops = {
	.write = send_write,
};

static const struct file_operations recv_fops = {
	.read = recv_read,
};

static const struct file_operations count_fops = {
	.read = count_read,
};

static int cv1800_mbox_demo_probe(struct platform_device *pdev)
{
	struct cv1800_mbox_demo *demo;
	struct device *dev = &pdev->dev;

	demo = devm_kzalloc(dev, sizeof(*demo), GFP_KERNEL);
	if (!demo)
		return -ENOMEM;

	demo->mbox_cl.dev = dev;
	demo->mbox_cl.tx_block = true;
	demo->mbox_cl.tx_tout = 1000;
	demo->mbox_cl.knows_txdone = false;
	demo->mbox_cl.rx_callback = mbox_rx_callback;

	demo->mbox_chan = mbox_request_channel_byname(&demo->mbox_cl, CHAN_NAME);
	if (IS_ERR(demo->mbox_chan))
		return dev_err_probe(dev, PTR_ERR(demo->mbox_chan),
				     "Failed to request channel '%s'\n", CHAN_NAME);

	platform_set_drvdata(pdev, demo);

	demo->debugfs_dir = debugfs_create_dir("cv1800_mbox_demo", NULL);
	debugfs_create_file("send", 0200, demo->debugfs_dir, demo, &send_fops);
	debugfs_create_file("recv", 0400, demo->debugfs_dir, demo, &recv_fops);
	debugfs_create_file("count", 0400, demo->debugfs_dir, demo, &count_fops);

	dev_info(dev, "mailbox demo loaded (channel 0 -> CPU 1 self-send)\n");
	return 0;
}

static void cv1800_mbox_demo_remove(struct platform_device *pdev)
{
	struct cv1800_mbox_demo *demo = platform_get_drvdata(pdev);

	debugfs_remove_recursive(demo->debugfs_dir);
	mbox_free_channel(demo->mbox_chan);
	dev_info(&pdev->dev, "mailbox demo unloaded\n");
}

static const struct of_device_id cv1800_mbox_demo_of_match[] = {
	{ .compatible = "cv1800-mbox-demo" },
	{},
};
MODULE_DEVICE_TABLE(of, cv1800_mbox_demo_of_match);

static struct platform_driver cv1800_mbox_demo_driver = {
	.driver = {
		.name = "cv1800-mbox-demo",
		.of_match_table = cv1800_mbox_demo_of_match,
	},
	.probe = cv1800_mbox_demo_probe,
	.remove = cv1800_mbox_demo_remove,
};

module_platform_driver(cv1800_mbox_demo_driver);

MODULE_DESCRIPTION("cv1800 mailbox demo driver");
MODULE_LICENSE("GPL");

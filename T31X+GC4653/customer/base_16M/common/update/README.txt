#cdc固件升级配置:

1.root-uclibc-1.1.tar.gz
	ramdisk内核需要的rootfs包,其解压需要root权限

2.mknod.sh
	一键解压脚本

3.内核中的配置文件
	arch/mips/config/kiva_uvc_ramdisk_defconfig
		CONFIG_INITRAMFS_SOURCE="解压后的文件路径"

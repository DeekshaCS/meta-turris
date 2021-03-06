# Support for device tree generation
FILES_kernel-devicetree = "/${KERNEL_IMAGEDEST}/devicetree*"

PACKAGES_append_morty = " kernel-devicetree"

normalize_dtb () {
	DTB="$1"
	if echo ${DTB} | grep -q '/dts/'; then
		bbwarn "${DTB} contains the full path to the the dts file, but only the dtb name should be used."
		DTB=`basename ${DTB} | sed 's,\.dts$,.dtb,g'`
	fi
	echo "${DTB}"
}

get_real_dtb_path_in_kernel () {
	DTB="$1"
	DTB_PATH="${B}/arch/${ARCH}/boot/dts/${DTB}"
	if [ ! -e "${DTB_PATH}" ]; then
		DTB_PATH="${B}/arch/${ARCH}/boot/${DTB}"
	fi
	echo "${DTB_PATH}"
}

do_compile_append() {
	for DTB in ${KERNEL_DEVICETREE}; do
		DTB=`normalize_dtb "${DTB}"`
		oe_runmake ${DTB}
	done
}

do_install_append() {
	for DTB in ${KERNEL_DEVICETREE}; do
		DTB=`normalize_dtb "${DTB}"`
		DTB_EXT=${DTB##*.}
		DTB_BASE_NAME=`basename ${DTB} ."${DTB_EXT}"`
		for type in ${KERNEL_IMAGETYPE_FOR_MAKE}; do
			symlink_name=${type}"-"${KERNEL_IMAGE_SYMLINK_NAME}
			DTB_SYMLINK_NAME=`echo ${symlink_name} | sed "s/${MACHINE}/${DTB_BASE_NAME}/g"`
			DTB_PATH=`get_real_dtb_path_in_kernel "${DTB}"`
			install -m 0644 ${DTB_PATH} ${D}/${KERNEL_IMAGEDEST}/devicetree-${DTB_SYMLINK_NAME}.${DTB_EXT}
		done
	done
}

do_deploy_append() {
	for DTB in ${KERNEL_DEVICETREE}; do
		DTB=`normalize_dtb "${DTB}"`
		DTB_EXT=${DTB##*.}
		DTB_BASE_NAME=`basename ${DTB} ."${DTB_EXT}"`
		for type in ${KERNEL_IMAGETYPE_FOR_MAKE}; do
			base_name=${type}"-"${KERNEL_IMAGE_BASE_NAME}
			symlink_name=${type}"-"${KERNEL_IMAGE_SYMLINK_NAME}
			DTB_NAME=`echo ${base_name} | sed "s/${MACHINE}/${DTB_BASE_NAME}/g"`
			DTB_SYMLINK_NAME=`echo ${symlink_name} | sed "s/${MACHINE}/${DTB_BASE_NAME}/g"`
			DTB_PATH=`get_real_dtb_path_in_kernel "${DTB}"`
			install -d ${DEPLOYDIR}
			install -m 0644 ${DTB_PATH} ${DEPLOYDIR}/${DTB_NAME}.${DTB_EXT}
			ln -sf ${DTB_NAME}.${DTB_EXT} ${DEPLOYDIR}/${DTB_SYMLINK_NAME}.${DTB_EXT}
		done
	done
}

pkg_postinst_kernel-devicetree () {
	cd /${KERNEL_IMAGEDEST}
	for DTB in ${KERNEL_DEVICETREE}; do
		for type in ${KERNEL_IMAGETYPE_FOR_MAKE}; do
			symlink_name=${type}"-"${KERNEL_IMAGE_SYMLINK_NAME}
			DTB_EXT=${DTB##*.}
			DTB_BASE_NAME=`basename ${DTB} ."${DTB_EXT}"`
			DTB_SYMLINK_NAME=`echo ${symlink_name} | sed "s/${MACHINE}/${DTB_BASE_NAME}/g"`
			update-alternatives --install /${KERNEL_IMAGEDEST}/${DTB_BASE_NAME}.${DTB_EXT} ${DTB_BASE_NAME}.${DTB_EXT} /boot/devicetree-${DTB_SYMLINK_NAME}.${DTB_EXT} ${KERNEL_PRIORITY} || true
		done
	done
}

pkg_postrm_kernel-devicetree () {
	cd /${KERNEL_IMAGEDEST}
	for DTB in ${KERNEL_DEVICETREE}; do
		for type in ${KERNEL_IMAGETYPE_FOR_MAKE}; do
			symlink_name=${type}"-"${KERNEL_IMAGE_SYMLINK_NAME}
			DTB_EXT=${DTB##*.}
			DTB_BASE_NAME=`basename ${DTB} ."${DTB_EXT}"`
			DTB_SYMLINK_NAME=`echo ${symlink_name} | sed "s/${MACHINE}/${DTB_BASE_NAME}/g"`
			update-alternatives --remove ${DTB_BASE_NAME}.${DTB_EXT} /boot/devicetree-${DTB_SYMLINK_NAME}.${DTB_EXT} ${KERNEL_PRIORITY} || true
		done
	done
}



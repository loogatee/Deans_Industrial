
# dart6ul_UbuntuBase
 &nbsp; &nbsp; &nbsp; &nbsp; Build Ubuntu Base from scratch for dart6ul

### Follow Variscite instructions 1st

Goto: http://variwiki.com/index.php?title=Debian_Build_Release

Perform all the steps up to number 3.2.3: 'Build rootfs'.

**You do not build the rootfs**.   The last step from the list to execute is this:

    `sudo ./make_var_mx6ul_dart_debian.sh -c modules`

### Execute the make_rootfs.sh shell script.

This will build rootfs locally.

### Put a sdcard in your computer

I think a 4GB card will work.  I typically use 16.<br>
The card will be overwritten.

### Execute the make_sdcard.sh shell script.

This will put the entire Linux distro on the card, and make the card bootable.

### Place sdcard into slot on cdc3 board.

And reboot

### Login as root

### Execute 1stboot_config.sh

reboot

### Hit the webserver:  Get A/D values

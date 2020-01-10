
# dart6ul_UbuntuBase
 &nbsp; &nbsp; &nbsp; &nbsp; Build Ubuntu Base from scratch for dart6ul

1. ### Follow Variscite instructions 1st

<div style="text-indent: 100em;">
Goto: http://variwiki.com/index.php?title=Debian_Build_Release

Perform all the steps up to number 3.2.3: 'Build rootfs'.

**You do not build the rootfs**.   The last step from the list to execute is this:

    `sudo ./make_var_mx6ul_dart_debian.sh -c modules`
</div>

2. ### Execute the make_rootfs.sh shell script.

<div style="margin-left: 10em;">
This will build rootfs locally.
</div>

3. ### Put a sdcard in your computer

<div style="margin-left: 10em;">
I think a 4GB card will work.  I typically use 16.<br>
The card will be overwritten.
</div>

4. ### Execute the make_sdcard.sh shell script.

<div style="margin-left: 10em;">
This will put the entire Linux distro on the card, and make the card bootable.
</div>

5. ### Place sdcard into slot on cdc3 board.

<div style="margin-left: 10em;">
And reboot
</div>

6. ### Login as root

7. ### Execute 1stboot_config.sh

<div style="margin-left: 10em;">
reboot
</div>

8. ### Hit the webserver:  Get A/D values

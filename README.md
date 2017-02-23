# orionssagx2camera
TheSkyX Linux camera plugin for Orion SSAG Autoguider  

USB Permissions  
---------------  
Open /etc/udev/rules.d/10-SoftBisque.rules and add below entry in to file.  
 
##############################################################  
# ORION SSAG  
ATTRS{idVendor}=="1856", ATTRS{idProduct}=="0011", MODE:="0666"  
ATTRS{idVendor}=="1856", ATTRS{idProduct}=="0012", MODE:="0666"  


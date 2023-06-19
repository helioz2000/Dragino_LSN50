LoRa STM32 Source Code, repackaged to build with STM32CubeIDE
=============================================================
This is a copy of the repository [LoRa_STM32](https://github.com/dragino/LoRa_STM32) from Dragino. 
The repository contains the sources to build a sample LoRa application for their [LSN50](http://www.dragino.com/products/lora/item/128-lsn50.html) and [LSN50-v2](https://www.dragino.com/products/lora-lorawan-end-node/item/155-lsn50-v2.html).

The code itself is unmodified, [Christophe](https://https://github.com/cthil) has added project defintions and settings to build with STM32CubeIDE.
I have tested this with the LSN50-v2 and STM32CubeIDE 1.12.1

In order to import the project, open STM32CubeIDE, then select **File->Import->Existing Projects** and click **Next**, then **Browse** and select path for the repo directory. Then click **Finish** to import the project into your workspace.

Original versions:
- V1.7.2 by [Christophe](https://https://github.com/cthil)
- V1.8.0 by [helioz2000](https://https://github.com/helioz2000)

19/06/2023 - V1.8.0
- changed processor to L072CBTx to reflect correct Flash size of 128K (previously 192K)
- fixed compiler warnings, no functional code changes, same version number
- added Debug and Release launch configurations

Define Target Device:
=====================
`.inc/hw_conf.h` contains definition for target board, either [LSN50-v2](https://www.dragino.com/products/lora-lorawan-end-node/item/155-lsn50-v2.html) or [LoRa ST Module](https://www.dragino.com/products/lora/item/127-lora-st.html).


To change LoRa region:
======================
REGION_AU915 definition in *Menu->Project->Properties->C/C++Build->Settings->MCU GCC Compiler->Preprocessor*
remember to change for both **Debug** and **Release**.


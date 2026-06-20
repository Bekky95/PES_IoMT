# Projektarbeit PES - IoMT

## Inhaltsverzeichnis

- [Handout](#handout)
    - [Pinbelegung](#pinbelegung)
    - [Shell Commands](#shell-commands)
- [Notizen](#notizen)

## Handout

### Pinbelegung

<table>
<tr>
<th>
<img src="./images/STM_Schema.png"width="400"> 
<img src="./images/ESP32_Schema.png"width="400">
</th>
<th style="text-align:left;">

- STM-ADC1 → Sensor
- STM-ADC2 → Sensor
- STM-ADC4 → Sensor
- STM-I²C → Sensor Pulsoximeter
- STM-USART2 → ESP-UART2
</th>
</tr>
</table>

### Shell Commands

```shell
git clone https://github.com/Bekky95/PES_IoMT.git 
```

## Notizen

Beim initialen push hab ich einfach mal alles hochgeladen
-> ein gitignore fehlt noch 

Workflow:
-TouchGFX:
 -UI design tool that allows drag and drop design and generates the code for us
 -https://www.st.com/en/development-tools/touchgfxdesigner.html#section-get-software-table
 
-STM32CubeMX:
 -Configures hardware and generates init code
 -https://www.st.com/en/development-tools/stm32cubemx.html#
  
-STM32CubeIDE:
 -STMs IDE works well with other tools
 -https://www.st.com/en/development-tools/stm32cubeide.html
  

**![Arduino UNO+WiFi R3 ATmega328P+Node MCU ESP8266 CH340G Compatible Board](https://robocraze.com/cdn/shop/products/ezgif.com-gif-maker_92.jpg?v=1670585294)**

Here are the steps to set up your **Arduino UNO+WiFi R3 (ATmega328P+Node MCU ESP8266)** in the Arduino IDE:

1.  **Open Arduino IDE**: Launch the Arduino Integrated Development Environment (IDE) on your computer.
2.  **Access Preferences**: Go to the menu and select 'File' -> 'Preferences'.
3.  **Add ESP8266 URL**: In the 'Settings' tab, find the 'Additional Boards Manager URLs:' text box and enter the following URL: **http://arduino.esp8266.com/stable/package_esp8266com_index.json**. Click 'Ok' to save.
4.  **Open Board Manager**: Go to 'Tools' -> 'Board' -> 'Boards Manager'.
5.  **Search for ESP8266**: In the Boards Manager window, type 'esp8266' in the search bar.
6.  **Install ESP8266 Package**: Find the 'ESP8266 by ESP8266 Community' entry and click 'Install'.
7.  **Close the Boards Manager**: After installation, close the Boards Manager window.

Your Arduino IDE is now set up to work with the ESP8266 board.

![MEGA 2560 Mega+WiFi R3 ATmega2560+ESP8266 32M Memory USB-TTL CH340G  Compatible Mega NodeMCU ESP8266 : Amazon.in: Computers & Accessories](https://m.media-amazon.com/images/I/41g2Rsvr49L.jpg)

## **Arduino Mega 2560+WiFi R3 ATmega2560+Node MCU ESP8266 CH340G**

**Here are the steps to set up your Arduino Mega 2560 board in the Arduino IDE:**

1.  **Open Arduino IDE**: Launch the Arduino Integrated Development Environment (IDE) on your computer.
2.  **Open the Board Manager**: Go to 'Tools' -> 'Board' -> 'Boards Manager'.
3.  **Search for Arduino AVR Boards**: In the Boards Manager window, type 'Arduino AVR Boards' in the search bar.
4.  **Install the Arduino AVR Boards Package**: Find the 'Arduino AVR Boards' entry and click 'Install'.
5.  **Select the Arduino Mega 2560 Board**: Go to 'Tools' -> 'Board' and select 'Arduino Mega or Mega 2560' from the list.
6.  **Select the Correct Processor**: Go to 'Tools' -> 'Processor' and ensure 'ATmega2560 (Mega 2560)' is selected.
7.  **Select the Correct Port**: Go to 'Tools' -> 'Port' and select the port to which your Arduino Mega 2560 is connected.
8.  Your Arduino IDE is now configured to work with the Arduino Mega 2560 board.

**Here are the steps to set up the ESP8266 board in your Arduino IDE:**

1.  **Open Arduino IDE**: Launch the Arduino Integrated Development Environment (IDE) on your computer.
2.  **Go to Preferences**: Select 'File' -> 'Preferences' from the menu.
3.  **Add ESP8266 URL**: In the 'Settings' tab, enter the following URL into the 'Additional Boards Manager URLs:' text box: **http://arduino.esp8266.com/stable/package_esp8266com_index.json**. Click 'Ok'.
4.  **Open Boards Manager**: Go to 'Tools' -> 'Board' -> 'Boards Manager'.
5.  **Search for ESP8266**: Type 'esp8266' in the search bar.
6.  **Install ESP8266 Package**: Click on 'ESP8266 by ESP8266 Community' and then click 'Install'.
7.  **Close Boards Manager**: Once the installation is complete, close the Boards Manager.

Your Arduino IDE is now ready to use with the ESP8266 board.

---

After setting up the board, follow the same steps to upload code for the following configurations:

1.  **Arduino UNO+WiFi R3 ATmega328P+Node MCU ESP8266**
2.  **Arduino Mega 2560+WiFi R3 ATmega2560+Node MCU ESP8266 CH340G**

These steps ensure that your code is correctly uploaded to the respective board.

![](https://33333.cdn.cke-cs.com/kSW7V9NHUXugvhoQeFaf/images/a56ca46527e20b4b7944abcafa202a135744f425ec1cfc32.png)

### **Upload Code to Arduino Uno**

1.  **Write the Code**: Open the Arduino IDE and write your code.
2.  **Set Switches**: On the board, ensure only switches 3 and 4 are ON.
3.  **Select Board**: In the Arduino IDE, go to 'Tools' -> 'Board' -> 'Boards Manager', find 'Arduino AVR Boards', and select 'Arduino Uno'.
4.  **Select Port**: In the Arduino IDE, go to 'Tools' -> 'Port' and select the correct port for the Arduino Uno.
5.  **Upload Code**: Click the 'Upload' button to upload your code to the Arduino Uno.

### **Upload Code to ESP8266**

1.  **Write the Code**: Open the Arduino IDE and write your code.
2.  **Set Switches**: On the board, ensure only switches 5, 6, and 7 are ON.
3.  **Select Board**: In the Arduino IDE, go to 'Tools' -> 'Board' -> 'Boards Manager', find 'esp8266', and select 'Generic ESP8266 Module'.
4.  **Select Port**: In the Arduino IDE, go to 'Tools' -> 'Port' and select the correct port for the ESP8266.
5.  **Upload Code**: Click the 'Upload' button to upload your code to the ESP8266.
6.  **Reset**: After uploading, press the reset button on the ESP8266.
7.  **Check Output**: To check your code's output, turn OFF switch 7.

### **Combining Both Boards**

1.  **Upload Code to Arduino**: Follow the steps above to upload your code to the Arduino.
2.  **Upload Code to ESP8266**: Follow the steps above to upload your code to the ESP8266.
3.  **Set Switches for Combined Use**: Turn OFF all switches and turn ON switches 1 and 2.
4.  **Communication**: Use the Serial Monitor in the Arduino IDE to communicate between both boards.

**Note**: When both boards are in use, the Serial Monitor in the Arduino IDE may not display correctly due to the shared serial communication.

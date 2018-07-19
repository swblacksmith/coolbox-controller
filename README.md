# coolbox-controller

A [Trinket5V](https://www.adafruit.com/product/1501) based controller for a coolbox, that is supplied by a solar-panel with limited battery capacity. The coolbox uses approx. 50W at 12V and is able to cool approx. 14°C relative to the surrounding temperature. A solar panel is mounted on an allotment hut, and is connected to a set of batteries with a dedicated charge-controller, and also powers a couple of LEDs (3-6W) for lighting up the hut. 

The problem is, that the batteries are quite quickly drained when the sun goes down, thereby also hindering the LEDs from working. Also there is a limitation on how long the coolbox is allowed to run continuously. 

To alleviate this problem, this controller will attempt to reduce the running time of the coolbox in the hours with less sunlight. This will unfortunately mean that the temperature of the contents might not reach the recommended temperature for storing food. However the food is only stored for a short while, and it is considered preferably to have some cooling during the night compared to none.

The setup is a [Trinket5V](https://www.adafruit.com/product/1501) connected to an analog light sensor module on A1/D2, a relay module with opto-coupler on D0, and using the on-board LED on D1 for signaling the state of the controller. The board is supplied with 12V from the charge-controller output, and the Trinket onboard power regulator adjusts it to 5V, which is then used for the light sensor module and relay module. A fuse is placed before the entire setup, dimensioned for the coolbox.
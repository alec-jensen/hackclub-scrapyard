from machine import Pin
import utime
import random

ledPins = [Pin(0, Pin.OUT), Pin(1, Pin.OUT), Pin(2, Pin.OUT), Pin(3, Pin.OUT)]
btnPins = [Pin(6, Pin.IN, Pin.PULL_UP), Pin(7, Pin.IN, Pin.PULL_UP), Pin(8, Pin.IN, Pin.PULL_UP), Pin(9, Pin.IN, Pin.PULL_UP)]

pattern = []

def startSimon(scoreRequired = 10):
    global pattern
    pattern = []
    score = 0
    while score < scoreRequired:
        pattern.append(random.randint(0,3))
        displayPattern()
        if not userInput():
            for _ in range(3):
                for pin in ledPins:
                    pin.on()
                utime.sleep_ms(333)
                for pin in ledPins:
                    pin.off()
                utime.sleep_ms(333)
            return False
        score += 1
        utime.sleep(1)
    return True



def displayPattern():
    for light in pattern:
        ledPins[light].on()
        utime.sleep_ms(170)
        ledPins[light].off()
        utime.sleep_ms(130)

def userInput():
    for light in pattern:
        activeButtons = []
        while len(activeButtons) < 1:
            activeButtons = []
            for i in range(4):
                if btnPins[i].value() == False:
                    activeButtons.append(i)
        utime.sleep_ms(10)
        if activeButtons[0] == light and len(activeButtons) <= 1:
            ledPins[light].on()
        else:
            return False
        while len(activeButtons) > 0:
            activeButtons = []
            for i in range(4):
                if btnPins[i].value() == False:
                    activeButtons.append(i)
                    utime.sleep_ms(10)
        ledPins[light].off()
    return True

if __name__ == "__main__":
    startSimon(2)
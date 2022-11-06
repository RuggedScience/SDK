# Getting started
Below are examples on how to use the rssdk Python module.
For more information about the available APIs see the [librsdio](/librsdio.md) and [librspoe](/librspoe.md) docs.
If you run into any issues be sure to check the [troubleshooting](/README.md#troubleshooting) section first.


# Dio Example
```python
from rssdk import RsDio, OutputMode

dio = RsDio()
dio.setXmlFile("ecs9000.xml")

dio.setOutputMode(1, OutputMode.ModeNpn)

dio.digitalRead(1, 1)
dio.digitalWrite(1, 11, True)

```

# PoE Example
```python
from rssdk import RsPoe, PoeState

poe = RsPoe()
poe.setXmlFile("ecs9000.xml")

poe.getPortState(3)
poe.setPortState(PoeState.StateDisabled)
```

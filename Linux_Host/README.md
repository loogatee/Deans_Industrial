
---

# LinuxComms

Communicates with the *Controls Processor*.

 - Packet-based RS-232 driver on the back-end
 - Message Queue (ZMQ) on the front-end
 - Has an endpoint for packets originating on *Controls*

The physical device for comms on the *Linux side* is /dev/ttymxc1

The physical device for comms on the *Controls side* is USART2

---

# luafcgid

This is the bridge that allows the Web-server to execute Lua code due
to http GETS and PUTS.


---





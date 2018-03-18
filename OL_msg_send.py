import bitcoin as btc
import ethereum as eth 
import json
import hashlib
import requests

# Message definition#
jstring = {’app id’=1, message’=’Hello world’, ’client’=1}
msg = json .dumps( jstring , sort keys=True)
hash function = hashlib.sha256()
hash function.update(msg)
hash message = hash function . digest

r = requests . post ( server address , json=msg)

eth.add msg out of the chain(hash message)
btc.add msg out of the chain(hash message)
btc.add msg out of the chain(hash message, mode=’OPRETURN’)

#Message Send#


import quant . app   builder as q
app = new q. client application ()
#in conf . txt there are all the information #about the Filtering Layer

app.set configuration(./conf.txt)
jstring = {’app id’=1, message’=’Hello world’}

msg = json .dumps( jstring ) app . sendmessage (msg)

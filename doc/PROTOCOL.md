# nanofiles-c protocol specification

### Notation

 - asdf: static data
 - \< >: mandatory field
 - \[ ]: optional field
 - \[...]: last object N times
 - All sizes are in bytes

## Directory protocol

UTF-8 encoded text/plain in key:value format over simple UDP

### Client requests

#### Ping request

Test the connection and compatibility

 - Operation `ping`
 - Fields
   - `protocol id`
 - Answer 'ping reply'

```
operation:ping
[protocol:<protocol id>]

```

#### Filelist request

Get file information known by directory

 - Operation `filelist`
 - Fields: None
 - Answer 'filelist reply [bad]'

```
operation:filelist

```

#### Publish requst

Inform directory of list available files for download from client

 - Operation `publish`
 - Fields: (see below), the list can be empty
 - Answer 'publish response'

```
operation: publish
[port: <port>]
<filename1>: <size1>; <hash1>
<filename2>: <size2>; <hash2>
[...]

```


### Directory responses

#### Ping reply

Acknoledges back connection and informs client of compatibility

 - Operation `pingOk`
 - Fields: None
 - Answer to 'ping request'

```
operation:pingOk

```

#### Filelist reply

Send back list of known (filename, hash, size and peers) for every file known

 - Operation `filelistRes`
 - Fields: (see below), the list can be empty
 - Answer to 'filelist request'

```
operation:filelistRes
<filename1>:<size1>;<hash1>;<server1a>,<server1b>[...]
<filename2>:<size2>;<hash2>;<server2a>,<server2b>[...]
[...]

```

#### Filelist reply bad

Inform client of an error condition on the directory

 - Operation `filelistBad`
 - Fields: None
 - Answer to 'filelist request'

```
operation:filelistBad

```

#### Publish reply

Acknowledge publish request

 - Operation `publishAck`
 - Fields: None
 - Answer to: 'publish request'

```
operation:publishAck

```

## Peer protocol

Binary little-endian over TCP buffers

All messages begin with an opcode byte

### Client requests

#### File request

Requests availability of file to be downloaded

 - Opcode: 0x02
 - Fields: 
   - fnamelen: Filename length
 - Answer: 'accepted' or 'file not found error'

```
0        1         byte
+--------+--------+
| opcode |fnamelen|
+-----------------+
| filename        |
| ...             |
```

#### Get chunk

Requests availability of file to be downloaded

 - Opcode: 0x04
 - Fields: 
   - offset[8]: Starting byte of chunk
   - size[4]: Size of chunk
 - Answer: 'chunk served'

```
0        1       2       3       4       5       6       7       8         byte
+--------+----------------------------------------------------------------+
| opcode | offset                                                         |
+--------+----------------------+-----------------------------------------+
| size                          |
+-------------------------------+
```

#### Stop download

 - Opcode: 0x05
 - Fields: None
 - Answer: None

```
0               
+--------+
| opcode |
+--------+
```

### Server requests

#### File not found error

 - Opcode: 0x01
 - Fields: None
 - Answer to: 'file request'

```
0               
+--------+
| opcode |
+--------+
```

#### Accepted

 - Opcode 0x03
 - Fields: None
 - Answer to: 'file request'

```
0               
+--------+
| opcode |
+--------+
```

#### Chunk served

Requests availability of file to be downloaded

 - Opcode: 0x06
 - Fields: 
   - size[4]: Size of chunk
   - data: 
 - Answer to: 'get chunk'

```
0        1       2       3       4       byte
+--------+------------------------------+
| opcode | size                         |
+--------+------------------------------+
| data ...
|
```



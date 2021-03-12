#!/usr/bin/env python3
import os
import protobuf.try_pb2 as T

if __name__ == '__main__' :
    t = T.Try()
    t.data.append("a")
    with open(os.path.join("", "try.pb"), "wb") as f:
        f.write(t.SerializeToString())

    with open(os.path.join("", "try.pb"), "rb") as f:
        t1 = T.Try()
        t1.ParseFromString( f.read() )
        print( t1.data[0] )

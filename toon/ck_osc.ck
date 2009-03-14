// chuck FILENAME

//  the patch
SinOsc s => JCRev r => dac;
.5 => s.gain;
.1 => r.mix;

//  create our OSC receiver
OscRecv recv;
17777 => recv.port;
recv.listen();

//  create an address in the receiver, store in new variable
recv.event("/ping") @=> OscEvent oe;
//  recv.event("/note,i") @=> OscEvent oe;

//  <<< "adding /test/note,i OSC handler""" >>>;

// infinite event loop
while (true)
{
    // wait for event to arrive
    oe => now;

    // grab the next message from the queue. 
    while( oe.nextMsg() )
    { 
        int i;
        float f;

        <<< "got ping ! " >>>;

        1.0 => f;

        // getFloat fetches the expected float (as indicated by "i f")
        oe.getInt() => i => Std.mtof => s.freq;
        // oe.getFloat() => f => s.gain;

        // print
        <<< "got OSC :", i >>>; // f
    }
}
// TODO : sporl shreds...


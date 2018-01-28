// Exported interface from aa3blast.dll.
// This allows us to pass into aa3blast an abstract blast source.
// This is used to blast from memory copied from the SCIT client
// over to the server.

struct Blast_input {
    virtual int version() = 0;
    virtual int read(void *buf, unsigned amount) = 0;
    // Side effect of size() is to reset the pointer.
    virtual int size() = 0;
    // Tell whether the input has run out.
    virtual int eof() = 0;
    virtual const char *name() = 0;
    };

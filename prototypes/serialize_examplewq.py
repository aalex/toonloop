from toon import serialize

if __name__ == '__main__':
    class Conf(object):
        def __init__(self):
            self.test_int = 2
            self.test_string = "egg"
            self.test_float = 3.14159
    
    conf = Conf()
    
    filename = 'simple_data_test.txt'
    try:
        conf.__dict__.update(serialize.load(filename))
        print 'loaded objects', conf.__dict__
    except serialize.SerializeError, e:
        print 'error: could not load file', e
    serialize.save(filename, conf.__dict__)
    print 'saved data'
    

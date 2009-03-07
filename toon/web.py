#!/usr/bin/env python
#
# ToonLoop for Python
#
# Copyright 2008 Tristan Matthews & Alexandre Quessy
# <le.businessman@gmail.com> & <alexandre@quessy.net>
#
# Original idea by Alexandre Quessy
# http://alexandre.quessy.net
#
# ToonLoop is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ToonLoop is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the gnu general public license
# along with ToonLoop.  If not, see <http://www.gnu.org/licenses/>.
#

from time import strftime

from twisted.web import http

_data = {
    'frame_rate':12,
    'enable_auto':False,
    'file_prefix':'toonloop_'
    }

def renderHomePage(request):
    """
    Handles get data and generates a web form.
    """
    global _data

    successful = False
    datetime = strftime("%Y-%m-%d at %H:%M:%S")

    if len(request.args) > 0:
        print "POST: ----------------"
        # POST data handling:
        # first process checkboxes:
        has_arg_enable_auto = False
        # next, process int and strings:
        
        _data['enable_auto'] = request.args.has_key('enable_auto')
        for key, words in request.args.items():
            try:
                value = words[0]
                print "Arg: %s = %s" % (key, value)
                if _data.has_key(key):
                    # TODO: validation
                    # casting
                    if type(_data[key]) is bool:
                        pass 
                    elif type(_data[key]) is int:
                        _data[key] = int(value)
                    elif type(_data[key]) is str:
                        _data[key] = value
            except Exception, e:
                print "ERROR parsing argument", key, words, e.message

        successful = True
    # Web form generation:
    request.write("""
    <html>
    <head>
      <title>ToonLoop Web Interface</html>
    </head>
    <body>
      <h1>ToonLoop</h1>
      <form action='/' method='get'>
        <p>
          File name prefix:
          <input type='text' name='file_prefix' value='%s' />
        </p>
        <p>
          Frame rate:
          <input type='text' name='frame_rate' value='%d' />
        </p>
        <p>
        Options : 
    """ % (_data['file_prefix'], _data['frame_rate']))
    checked_str = ''
    if _data['enable_auto']:
        checked_str = 'checked="checked"'
    request.write(
        "<input type='checkbox' name='enable_auto' value='True' %s/>%s<br />" % (checked_str, 'Enable auto frames grabbing. (intervalometer - timelapse)'))
    request.write("""
        </p>
        <input type='submit' />
        <input type='reset' />
      </form>""")
    if successful:
        request.write("""
        <p>
        Saved changes on %s.
        </p>
        """ % (datetime))
    request.write("""
    </body>
    </html>
    """)
    #        "<input type='radio' name='color' value='%s'>%s<br />" % (
    request.finish()

# def handlePost(request):
#     for key, values in request.args.items():
#         request.write("<h2>%s</h2>" % key)
#         request.write("<ul>")
#         for value in values:
#             request.write("<li>%s</li>" % value)
#         request.write("</ul>")
# 
#     request.write("""
#        </body>
#     </html>
#     """)
#     request.finish()

class ToonHttpRequestHandler(http.Request):
    pageHandlers = {
        '/': renderHomePage # ,
        #'/posthandler': handlePost,
        }
    
    def process(self):
        self.setHeader('Content-Type', 'text/html')
        if self.pageHandlers.has_key(self.path):
            handler = self.pageHandlers[self.path]
            handler(self)
        else:
            self.setResponseCode(http.NOT_FOUND)
            self.write("<h1>Not Found</h1>Sorry, no such page.")
            self.finish()

class ToonHttp(http.HTTPChannel):
    requestFactory = ToonHttpRequestHandler

class ToonHttpFactory(http.HTTPFactory):
    protocol = ToonHttp

if __name__ == "__main__":
    from twisted.internet import reactor
    port = 8888
    print "Sarting web server on %d" % port
    reactor.listenTCP(port, ToonHttpFactory())
    reactor.run()


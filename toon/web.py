#!/usr/bin/env python
#
# Web Form Renderer
#
# Copyright 2009 Alexandre Quessy
# <alexandre@quessy.net>
#
# This file is part of ToonLoop.
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
import rss

class FormInput(object):
    """
    Three-column table-based layout of HTML form elements
    """
    def __init__(self, **kwargs):
        self.title = ''
        self.name = ''
        self.default = ''
        self.value = ''
        self.description = ''
        self.error_message = ''
        self.__dict__.update(**kwargs)
    
    def get_html(self):
        """
        Returns HTML for the form element
        """
        raise NotImplementedError()
        
    def _render_error(self):
        """
        Renders an error message into HTML. 
        In red 
        """
        error_mess = ''
        if self.error_message != '':
            error_mess = "<br />%s" % (self.error_message)
        return error_mess
    
    def set_value(self, value):
        """
        validates the GET/POST variable and changes it if valid. 
        :return boolean: if changed or not.
        """
        raise NotImplementedError()

    def parse_arguments(self, args):
        """
        Parses the post/get arguments to check for this value
        :return boolean: if changed or not.
        """
        changed = False
        if args.has_key(self.name):
            val = args[self.name]
# TODO : add multi words text support
            try:
                val = val[0]
                return self.set_value(val)
            except IndexError, e:
                self.error_message = "Invalid word index %s" % (e.message)
                return False

class OnOffInput(FormInput):
    """
    Radio buttons inputs for feature enabling/disabling
    """
    def __init__(self, **kwargs):
        FormInput.__init__(self, **kwargs)
        if self.default == '':
            self.default = True
        if self.value == '':
            self.value = True

    def get_html(self):
        error_mess = self._render_error()
        on_checked = ""
        off_checked =""
        if self.value is True:
            on_checked = " checked='checked'"
        else:
            off_checked = " checked='checked'"
        txt = """
        <tr>
            <td>%s</td>
            <td>
                <input type='radio' name='%s' value='true' %s /> On <br />
                <input type='radio' name='%s' value='false' %s /> Off
            </td>
            <td>%s %s</td>
        </tr>
        """ % (self.title, self.name, on_checked, \
            self.name, off_checked, \
            self.description, error_mess)
        return txt
        
    def set_value(self, value):
        """
        string either "true" of "false"
        :return boolean: if changed or not.
        """
        if value == "true":
            self.value = True
            return True
        elif value == "false":
            self.value = False
            return True
        else:
            self.error_message = "Error parsing boolean string " + value
            return False

class TextInput(FormInput):
    """
    'text' type HTML form input
    """
    def get_html(self):
        error_mess = self._render_error()
        txt = """
        <tr>
            <td>%s</td>
            <td><input type='text' name='%s' value='%s' /></td>
            <td>%s %s</td>
        </tr>
        """ % (self.title, self.name, self.value, self.description, error_mess)
        return txt
        
    def set_value(self, arg):
        """
        :return boolean: if changed or not.
        """
        self.value = str(arg)
        return True

class IntegerTextInput(TextInput):
    """
    'text' type HTML form input but for integers only.
    """
    def __init__(self, **kwargs):
        self.min = 0
        self.max = 999
        FormInput.__init__(self, **kwargs)
        if self.default == '':
            self.default = 0
        if self.value == '':
            self.value = 0

    def set_value(self, value):
        try:
            val = int(value)
        except ValueError:
            self.error_message = "Invalid integer %s" % (value)
            print self.error_message
            # and we keep the former value
            return False
        else:
            if val < self.min:
                self.error_message = "Minimum value is %s" % (self.min)
                return False
            elif val > self.max:
                self.error_message = "Maximum value is %s" % (self.max)
                return False
            else:
                self.value = val
                return True

class WebForm(object):
    """
    Wrapper for form elements formatted in a table.
    
    To add form items, use self.inputs.append(input)
    """
    def __init__(self, **kwargs):
        self.method = "post" # get
        self.target = "/" # aka action
        self.inputs = []
        self.__dict__.update(**kwargs)

    def get_now(self):
        """
        Returns a string representing a humand-readable date/time
        """
        return strftime("%Y-%m-%d at %H:%M:%S")

    def get_html(self, args):
        """
        Renders the web form and its elements to HTML
        
        :param args: dict of arguments. use request.args
        :return string: HTML
        """
        global _on_change
        txt = """ 
        <form action='%s' method='%s'>
        <table>
        """ % (self.target, self.method)
        for input in self.inputs:
            changed = False
            ok = False
            previous_value = input.value

            # print input
            input.error_message = ''
            changed = input.parse_arguments(args) # process get/post vars
            txt += input.get_html()
            if changed:
                ok = _on_change(input)
                if not ok:
                    input.value = previous_value 

        txt += """
        </table>
        <input type='submit' />
        <input type='reset' />
        </form>
        """
        return txt



# -------------------------------------------------
# DATA AND EXAMPLE

def default_on_change_callback(form_input):
    """
    Called when a form variable is changed. 
    """
    print "%s: %s" % (input.name, input.value)
    return True

# module variables
# type is class WebForm
single_web_form = None
_on_change = default_on_change_callback

def render_single_web_form(request):
    """
    Handles get/post data and generates a web form.
    """
    global single_web_form

    successful = False
    if len(request.args) > 0:
        print "POST: ", request.args
        successful = True
    txt = """
    <html>
    <head>
      <title>ToonLoop Web Interface</html>
    </head>
    <body>
      <h1>ToonLoop</h1>
    """
    txt += single_web_form.get_html(request.args)
    datetime = single_web_form.get_now()
    if successful:
        txt += """
        <p>
        Saved changes on %s.
        </p>
        """ % (datetime)
    txt += """
    </body>
    </html>
    """
    request.write(txt)
    request.finish()

def render_rss(request):
    """
    Generates a RSS feed.
    """
    txt = rss.test_rss()
    request.write(txt)
    request.finish()

class ToonHttpRequestHandler(http.Request):
    """
    Serves the web pages

    Fow now, only servers the pages:
     * the root ('/')
    """
    # VERY IMPOERTATN VAR
    pageHandlers = {
        '/':render_single_web_form, 
        '/rss':render_rss
        #'/': renderHomePage # ,
        #'/posthandler': handlePost,
        }
    
    def process(self):
        self.setHeader('Content-Type', 'text/html')
        if self.pageHandlers.has_key(self.path):
            handler = self.pageHandlers[self.path]
            handler(self)
        else:
            self.setResponseCode(http.NOT_FOUND)
            self.write("<h1>Not Found</h1>Sorry, no such page.<br />")
            self.write("<a href="/">Back to main.</a>")
            self.finish()

class ToonHttp(http.HTTPChannel):
    """
    Basic HTTP Twisted mandatory element.
    """
    requestFactory = ToonHttpRequestHandler

class ToonHttpFactory(http.HTTPFactory):
    """
    Basic HTTP Twisted mandatory element.
    """
    protocol = ToonHttp
def set_on_change_callback(callback):
    """
    Sets callabck for when a form value changes
    """
    global _on_change 
    _on_change = callback

def change_value(name, value):
    """
    Change the value of a form element
    """
    

def set_form(form):
    """
    Sets single HTML form to render. 
    """
    global single_web_form
    single_web_form = form

if __name__ == "__main__":
    from twisted.internet import reactor
    # from toon.web import ToonHttpFactory, set_form, set_on_change_callback
    
    port = 8080
    print "Sarting web server on %d" % port
    
    def on_change(input):
        """
        Called when a form variable is changed. 
        """
        print "%s: %s" % (input.name, input.value)
        return True

    web_form = WebForm(method="post")
    web_form.inputs = [
        TextInput(name="eg/g", title="eg/g", value="egg", \
            description="fill me"), 
        IntegerTextInput(name="spam", title="spam", value=9,\
            description="fill me"), 
        OnOffInput(name="ham", title="ham", value=False, \
            description="fill me")
    ]
# shot_num = [0, 9]
# file_name = "toonloop_"
# directory = "."
# sequence (shot)
# auto_rate 
# auto_enable False
# auto_allow True
    set_form(web_form)
    set_on_change_callback(on_change)

    reactor.listenTCP(port, ToonHttpFactory())
    reactor.run()


import xml.etree.ElementTree as ET
import json
from http.server import HTTPServer, BaseHTTPRequestHandler
import socketserver
from os import curdir, sep

j_file = '/tmp/tmp_hmd.json'
PORT = 8001

class MyHTTPReqHandler( BaseHTTPRequestHandler):

    def do_GET(self):

        with open(j_file) as f:
            ht_str = json.load(f)

        page = ET.Element('html')
        head = ET.SubElement(page, 'head')
        body = ET.SubElement(page, 'body')

        ln_css = ET.SubElement( head, 'link', \
                        {'rel': 'stylesheet', \
                        'href': 'style.css'} )
        meta = ET.SubElement(head, 'meta', {'charset': 'utf-8'})

        widg = ET.SubElement( body, 'article', { 'class': 'widget' } )
        tmpC = ET.SubElement( widg, 'div', {'class': 'temperature'} )
        tmp  = ET.SubElement( tmpC, 'span' )
        tmp.text = str(ht_str['temperature'])+'°'
        hmdC = ET.SubElement( widg, 'div', {'class': 'humidity'} )
        hmd  = ET.SubElement( hmdC, 'span' )
        hmd.text = str(ht_str['humidity'])+'%'

        basemnt = ET.SubElement( body, 'p' )
        cred    = ET.SubElement( basemnt, 'a' )
        cred.text = 'Inspired by: Serge Shu.'
        
        
        if self.path=="/":
          mimetype='text/html'
          self.send_response(200)
          self.send_header('Content-type',mimetype)
          self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
          self.send_header('Pragma', 'no-cache')
          self.send_header('Expires', '0')
          self.end_headers()  
          self.wfile.write( ET.tostring(page, method='html', encoding='unicode' ).encode() )
        if self.path.endswith(".css"):
          mimetype='text/css'
          self.send_response(200)
          self.send_header('Content-type',mimetype)
          self.end_headers()  
          f = open(curdir+sep+self.path)
          self.wfile.write(f.read().encode())
          f.close()
        if self.path.endswith('.ico'):
          self.send_response(204)
          self.end_headers()
       
httpd = HTTPServer(('',PORT), MyHTTPReqHandler)
httpd.serve_forever()



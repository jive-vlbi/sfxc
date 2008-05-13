#! /usr/bin/python 
import cgi
import os, sys
import cgitb; cgitb.enable()

CLUSTER_USER = "scariedm"
CLUSTER_PORT = "7779"
#CLUSTER_ADDR = "fs2.uva.science.das3.nl"
CLUSTER_ADDR = "localhost"
CLUSTER_ROOT_PATH = "scarie-bod"
CLUSTER_TMP_DIR = "tmp"
UPLOAD_DIR = "/tmp"

form = cgi.FieldStorage()

def save_uploaded_file (form_field, upload_dir):
    global form
    """This saves a file uploaded by an HTML form.
       The form_field is the name of the file input field from the form.
       For example, the following form_field would be "file_1":
           <input name="file_1" type="file">
       The upload_dir is the directory where the file will be written.
       If no file was uploaded or if the field does not exist then
       this does nothing.
    """
    sys.stderr.write("Begin of upload dd: \n")
    
    
    sys.stderr.write("GET STORAGE: \n")
    if not form.has_key(form_field):
      sys.stderr.write("NO keys@: \n")
      return
    
    sys.stderr.write("step2: \n")
    fileitem = form[form_field]
    
    if not fileitem.file: return
    if fileitem.filename == "": return      
    
    destfile = os.path.join(upload_dir, fileitem.filename)
    sys.stderr.write("Starting to upload: "+destfile+"\n")
    
    fout = file (destfile, 'wb')
    print "Uploading file: "+fileitem.filename
    while 1:
      chunk = fileitem.file.read(100000)
      if not chunk: break
      fout.write (chunk)
    fout.close()
    sys.stderr.write("File uploaded to: "+destfile+"\n")
    return (fileitem.filename, destfile)

print "Content-Type: text/html"
print ""

ctrlfile=save_uploaded_file("bod_ctrlfile", UPLOAD_DIR)
vexfile=save_uploaded_file("bod_vexfile", UPLOAD_DIR)

if ctrlfile and vexfile:
  print "<html>"
  print "<body'>"
  
  cpcmdctrl = "scp -P "+CLUSTER_PORT+" "+ctrlfile[1]+" "+CLUSTER_USER+"@"+CLUSTER_ADDR+":~/"+CLUSTER_ROOT_PATH+"/"+CLUSTER_TMP_DIR
  os.system(cpcmdctrl)
  
  cpcmdvex = "scp -P "+CLUSTER_PORT+" "+vexfile[1]+" "+CLUSTER_USER+"@"+CLUSTER_ADDR+":~/"+CLUSTER_ROOT_PATH+"/"+CLUSTER_TMP_DIR
  os.system(cpcmdvex) 
  
  startcmd = "ssh -p "+CLUSTER_PORT+" "+CLUSTER_USER+"@"+CLUSTER_ADDR+" 'cd scarie-bod && pwd && ./scarieBoD_reserve.py 20 tmp/"+ctrlfile[0]+"  tmp/"+vexfile[0]
  #ssh -p 7779 scariedm@localhost "cd scarie-bod && pwd && ./scarieBoD_reserve.py 20 tmp/n06c2_file_short.ctrl tmp/n06c2.vex" &
  
  os.system(startcmd+" &' &")  
  
  print cpcmdctrl+"<bR>"
  print cpcmdvex+"<bR>"
  print startcmd+"<bR>"
  print "Now you can go to the monitoring page !"
  
  print "</body>"
  print "</html>"
else:
  print "Files are missings"

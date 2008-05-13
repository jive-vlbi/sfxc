/*****************************************************************
 *
 * Damien Marchal UvA
 *
 *****************************************************************/
 
 
// global flag
var isIE = false;

// retrieve XML document (reusable generic function);
// parameter is URL string (relative or complete) to
// an .xml file whose Content-Type is a valid XML
// type, such as text/xml; XML source must be from
// same domain as HTML file
function loadXMLDoc(url, req, calfunc) 
{
    // branch for native XMLHttpRequest object
    if (window.XMLHttpRequest) {
        //document.write("ZUt")
        if( !req )
          req = new XMLHttpRequest();
       
        req.onreadystatechange = calfunc;
        //req.onprogress = onProgress
        
        req.open("POST", url, true);
        req.send(null);
        return req
    } else if (window.ActiveXObject) {
        isIE = true;
        req = new ActiveXObject("Microsoft.XMLHTTP");
        if (req) {
            req.onreadystatechange = calfunc;
            req.open("GET", url, false);
            req.send();
        }
        return req
    }
}

// retrieve XML document (reusable generic function);
// parameter is URL string (relative or complete) to
// an .xml file whose Content-Type is a valid XML
// type, such as text/xml; XML source must be from
// same domain as HTML file
function ajaxRequest(url, http_request, arguments, callback) 
{
    if (window.XMLHttpRequest) {
        if( !req )
          http_request = new XMLHttpRequest();
          
          if (http_request.overrideMimeType) {
         	 // set type accordingly to anticipated content type
           //http_request.overrideMimeType('text/xml');
           http_request.overrideMimeType('text/html');
          }
  
          http_request.onreadystatechange = callback;
          http_request.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
          http_request.setRequestHeader("Content-length", parameters.length);
          http_request.setRequestHeader("Connection", "close");
          http_request.send(parameters);
          http_request.open("POST", url, true);
          return http_request
    } else if (window.ActiveXObject) {
        isIE = true;
        req = new ActiveXObject("Microsoft.XMLHTTP");
        if (req) {
            alert("Not working with IE")
            req.onreadystatechange = calfunc;
            req.open("GET", url, false);
            req.send();
        }
        return req
    }
}

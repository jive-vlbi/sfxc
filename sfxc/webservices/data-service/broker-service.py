import os
import time
import web

import jsonrpclib.SimpleJSONRPCServer

os.environ['TZ'] = 'UTC'
time.tzset()

urls = (
    '/broker', 'broker'
)

app = web.application(urls, globals())

def transfer_data_completed(station, chunk, num_chunks, start, stop, dest):
    print station, chunk, num_chunks, start, stop, dest
    return "Success"

dispatcher = jsonrpclib.SimpleJSONRPCServer.SimpleJSONRPCDispatcher()
dispatcher.register_function(transfer_data_completed)

class broker:
    def POST(self):
        response = dispatcher._marshaled_dispatch(web.data())
        return response

    pass

if __name__ == "__main__":
    app.run()
    pass

CFLAGS = -I./ -I./serverFile -I./clientFile -I./share
server = server
client = client

$(server):
	g++ ./serverFile/server.cpp ./share/log.cpp -lpthread -g -o $@

$(client):
	g++ ./clientFile/client.cpp ./share/log.cpp -lpthread -g -o $@

clean:
	rm -rf $(server) $(client)

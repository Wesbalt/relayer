
code_dir := src
out_dir  := out

main_name   := main
client_name := client
test_name   := test

ip   := localhost
port := 5463

all: build

build:
	gcc $(code_dir)/$(main_name).cpp $(code_dir)/$(client_name).cpp -o $(out_dir)/$(client_name).exe -lstdc++ -lws2_32

run:
	./$(out_dir)/$(client_name).exe $(ip) $(port)

tbuild: # Build tests
	gcc $(code_dir)/$(test_name).cpp $(code_dir)/$(client_name).cpp -o $(out_dir)/$(test_name).exe -lstdc++ -lws2_32

trun: # Run tests
	./$(out_dir)/$(test_name).exe

clean:
	rm $(out_dir)/$(client_name).exe $(out_dir)/$(test_name).exe

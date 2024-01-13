search:
	g++ -fopenmp -std=c++17 -O2 -o essay-search ./*.cpp
search_no_parallel:
	g++ -std=c++17 -O2 -o essay-search ./*.cpp
query:
	./essay-search data query.txt output.txt
	diff output.txt query_output.txt
query_more:
	./essay-search data-more query_more.txt output.txt
	diff output.txt query_more_output.txt

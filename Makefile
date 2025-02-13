INCFLAGS = -I/usr/local/include/ -I./src/

CPP = g++
CPPFLAGS = -std=c++11 -g -O3 $(INCFLAGS) -fopenmp -ffast-math -Wall -Wno-strict-aliasing -lpthread
CPPFLAGSPG = -std=c++11 -g -O3 $(INCFLAGS) -fopenmp -Wall -Wno-strict-aliasing -lpthread -pg
LINKERFLAGS = -lz
LINKERFLAGSPG = -lz -pg
DEBUGFLAGS = -g -ggdb $(INCFLAGS)
HEADERS=$(shell find . -name '*.hpp')


all: apps tests 
apps: example_apps/connectedcomponents example_apps/pagerank example_apps/pagerank_functional example_apps/communitydetection example_apps/unionfind_connectedcomps example_apps/stronglyconnectedcomponents example_apps/trianglecounting example_apps/randomwalks example_apps/minimumspanningforest
als: example_apps/matrix_factorization/als_edgefactors  example_apps/matrix_factorization/als_vertices_inmem
tests: tests/basic_smoketest tests/bulksync_functional_test tests/dynamicdata_smoketest tests/test_dynamicedata_loader

echo:
	echo $(HEADERS)
clean:
	@rm -rf bin/*
	cd toolkits/collaborative_filtering/; make clean; cd ../../
	cd toolkits/parsers/; make clean; cd ../../
	cd toolkits/graph_analytics/; make clean; cd ../../

blocksplitter: src/preprocessing/blocksplitter.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) src/preprocessing/blocksplitter.cpp -o bin/blocksplitter $(LINKERFLAGS)

sharder_basic: src/preprocessing/sharder_basic.cpp $(HEADERS)
	@mkdir -p bin
	$(CPP) $(CPPFLAGS) src/preprocessing/sharder_basic.cpp -o bin/sharder_basic $(LINKERFLAGS)

example_apps/% : example_apps/%.cpp $(HEADERS)
	@mkdir -p bin/$(@D)
	$(CPP) $(CPPFLAGS) -Iexample_apps/ $@.cpp -o bin/$@ $(LINKERFLAGS) 

myapps/% : myapps/%.cpp $(HEADERS)
	@mkdir -p bin/$(@D)
	$(CPP) $(CPPFLAGS) -Imyapps/ $@.cpp -o bin/$@ $(LINKERFLAGS)

tests/%: src/tests/%.cpp $(HEADERS)
	@mkdir -p bin/$(@D)
	$(CPP) $(CPPFLAGS) src/$@.cpp -o bin/$@	$(LINKERFLAGS)


graphlab_als: example_apps/matrix_factorization/graphlab_gas/als_graphlab.cpp
	$(CPP) $(CPPFLAGS) example_apps/matrix_factorization/graphlab_gas/als_graphlab.cpp -o bin/graphlab_als $(LINKERFLAGS)

cf:
	cd toolkits/collaborative_filtering/; bash ./test_eigen.sh; 
	if [ $$? -ne 0 ]; then exit 1; fi
	cd toolkits/collaborative_filtering/; make 
cf_test:
	cd toolkits/collaborative_filtering/; make test; 
cfd:
	cd toolkits/collaborative_filtering/; make -f Makefile.debug

parsers:
	cd toolkits/parsers/; make
parsersd:
	cd toolkits/parsers/; make -f Makefile.debug
ga:
	cd toolkits/graph_analytics/; make
ta:
	cd toolkits/text_analysis/; make

docs: */**
	doxygen conf/doxygen/doxygen.config

######################Unicorn Specific (Do Not Change)###############
unicorn/% : unicorn/%.cpp $(HEADERS)
	@mkdir -p bin/$(@D)
	$(CPP) $(CPPFLAGS) -Iunicorn/ $@.cpp -o bin/$@ $(LINKERFLAGS)
#####################################################################
######################Unicorn Specific (Templates)################################################
swdebug: CPPFLAGS += -DSKETCH_SIZE=2000 -DK_HOPS=3 -DMEMORY -DPREGEN=10000 -DUSEWINDOW -DBASESKETCH -DDEBUG -g
swdebug: unicorn/main

sb: CPPFLAGS += -DSKETCH_SIZE=2000 -DK_HOPS=3 -DMEMORY -DPREGEN=10000 -g
sb: unicorn/main

######################Unicorn Toy Example################################################
toy:
	cd ../../data && mkdir -p train_toy
	number=0 ; while [ $$number -le 99 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/toy_data/base_train/base-toy-$$number.txt stream ../../data/toy_data/stream_train/stream-toy-$$number.txt decay 500 lambda 0.02 batch 2000 sketch ../../data/train_toy/sketch-toy-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/toy_data/base_train/base-toy-$$number.txt.* ; \
		rm -rf ../../data/toy_data/base_train/base-toy-$$number.txt_* ; \
		number=`expr $$number + 4` ; \
	done
	cd ../../data && mkdir -p test_toy
	number=300 ; while [ $$number -le 399 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/toy_data/base_test/base-attack-$$number.txt stream ../../data/toy_data/stream_test/stream-attack-$$number.txt decay 500 lambda 0.02 batch 2000 sketch ../../data/test_toy/sketch-attack-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/toy_data/base_test/base-attack-$$number.txt.* ; \
		rm -rf ../../data/toy_data/base_test/base-attack-$$number.txt_* ; \
		number=`expr $$number + 16` ; \
	done

streamspot_test: sb
	cd ../../data && mkdir -p train_toy
	number=0 ; while [ $$number -le 99 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/toy_data/base_train/base-toy-$$number.txt stream ../../data/toy_data/stream_train/stream-toy-$$number.txt decay 3000 lambda 0.02 batch 2000 sketch ../../data/train_toy/sketch-toy-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/toy_data/base_train/base-toy-$$number.txt.* ; \
		rm -rf ../../data/toy_data/base_train/base-toy-$$number.txt_* ; \
		number=`expr $$number + 4` ; \
	done
	cd ../../data && mkdir -p test_toy
	number=300 ; while [ $$number -le 399 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/toy_data/base_test/base-attack-$$number.txt stream ../../data/toy_data/stream_test/stream-attack-$$number.txt decay 3000 lambda 0.02 batch 2000 sketch ../../data/test_toy/sketch-attack-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/toy_data/base_test/base-attack-$$number.txt.* ; \
		rm -rf ../../data/toy_data/base_test/base-attack-$$number.txt_* ; \
		number=`expr $$number + 16` ; \
	done
	
evasion_mimicry_org_batch: sb
	cd ../../data && mkdir -p train_mimicry_evasion_org_batch
	number=0 ; while [ $$number -le 74 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/mimicry_data_parsed/base_train/mimicry-benign-$$number.txt stream ../../data/mimicry_data_parsed/stream_train/stream-benign-$$number.txt decay 3000 lambda 0.02 batch 2000 sketch ../../data/train_mimicry_evasion_org_batch/sketch-mimicry_benign-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/mimicry_data_parsed/base_train/mimicry-benign-$$number.txt.* ; \
		rm -rf ../../data/mimicry_data_parsed/base_train/mimicry-benign-$$number.txt_* ; \
		number=`expr $$number + 1` ; \
	done
	cd ../../data && mkdir -p test_mimicry_evasion_org_batch
	number=0 ; while [ $$number -le 99 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/mimicry_data_parsed/base_test/mimicry-evasion-$$number.txt stream ../../data/mimicry_data_parsed/stream_test/stream-evasion-$$number.txt decay 3000 lambda 0.02 batch 2000 sketch ../../data/test_mimicry_evasion_org_batch/sketch-evasion-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/mimicry_data_parsed/base_test/mimicry-evasion-$$number.txt.* ; \
		rm -rf ../../data/mimicry_data_parsed/base_test/mimicry-evasion-$$number.txt_* ; \
		number=`expr $$number + 1` ; \
	done
	
train_mimicry: sb
	cd ../../data && mkdir -p train_mimicry_evasion
	number=0 ; while [ $$number -le 74 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/mimicry_data_parsed/base_train/mimicry-benign-$$number.txt stream ../../data/mimicry_data_parsed/stream_train/stream-benign-$$number.txt decay 3000 lambda 0.02 batch 500 sketch ../../data/train_mimicry_evasion/sketch-mimicry_benign-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/mimicry_data_parsed/base_train/mimicry-benign-$$number.txt.* ; \
		rm -rf ../../data/mimicry_data_parsed/base_train/mimicry-benign-$$number.txt_* ; \
		number=`expr $$number + 1` ; \
	done

evasion_mimicry: sb
		cd ../../data && mkdir -p test_mimicry_evasion
	number=0 ; while [ $$number -le 99 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/mimicry_data_parsed/base_test/mimicry-evasion-$$number.txt stream ../../data/mimicry_data_parsed/stream_test/stream-evasion-$$number.txt decay 3000 lambda 0.02 batch 500 sketch ../../data/test_mimicry_evasion/sketch-evasion-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/mimicry_data_parsed/base_test/mimicry-evasion-$$number.txt.* ; \
		rm -rf ../../data/mimicry_data_parsed/base_test/mimicry-evasion-$$number.txt_* ; \
		number=`expr $$number + 1` ; \
	done

attack_mimicry: sb
	cd ../../data && mkdir -p test_mimicry_attack
	number=0 ; while [ $$number -le 99 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/mimicry_data_parsed/base_test/mimicry-attack-$$number.txt stream ../../data/mimicry_data_parsed/stream_test/stream-attack-$$number.txt decay 3000 lambda 0.02 batch 500 sketch ../../data/test_mimicry_attack/sketch-attack-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/mimicry_data_parsed/base_test/mimicry-attack-$$number.txt.* ; \
		rm -rf ../../data/mimicry_data_parsed/base_test/mimicry-attack-$$number.txt_* ; \
		number=`expr $$number + 1` ; \
	done

benign_mimicry: sb
	cd ../../data && mkdir -p test_mimicry_benign
	number=1 ; while [ $$number -le 24 ] ; do \
		bin/unicorn/main filetype edgelist base ../../data/mimicry_data_parsed/base_test/mimicry-benign-$$number.txt stream ../../data/mimicry_data_parsed/stream_test/stream-benign-$$number.txt decay 3000 lambda 0.02 batch 500 sketch ../../data/test_mimicry_benign/sketch-benign-$$number.txt chunkify 1 chunk_size 50 ; \
		rm -rf ../../data/mimicry_data_parsed/base_test/mimicry-benign-$$number.txt.* ; \
		rm -rf ../../data/mimicry_data_parsed/base_test/mimicry-benign-$$number.txt_* ; \
		number=`expr $$number + 1` ; \
	done


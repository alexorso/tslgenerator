# tslgenerator
TSL generator for the Category Partition Method

# How to Run

- Go to the folder corresponding to your operating system
- run `make`
- run `./tsl --manpage` to see usage and available options

Note: if you are running on a Linux machine, there seems to be a bug that causes a segmentation fault (See this [issue](https://github.com/snadi/tslgenerator/issues/3)). I applied a temp fix but seems this affected the case where you can supply an output file name instead of the default. For now, please just run as `./tsl inputfilename` which will output the results to a file called `inputfilename.tsl`.

# TODO
- Changing the parsing of the files so that properties can be used even if they are declared later in the file
- Providing a GUI for the tool


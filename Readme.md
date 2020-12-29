# Phone price prediction in parallel

## Description:
This project is phone price prediction using one layer neural network when we have test data and weights.

Code is in `c++` in its calculation is in parallel using `pthread` library.

Also serial code exist.

## How to use:

For running the project you just need to go in `parallel` or `serial` folder and run `make` like this:
```
make
```
Then there will be an executable file to run it you just need to run below command:
```
./PhonePricePrediction *datasetfiledirs*
```
When using parellel data set files should be in this format:
```
attributeColName1 attributeColName2 ... attributeColNamek result
value11           value12           ... value1k           resultClass1
.                 .                     .                 .
.                 .                     .                 .
.                 .                     .                 .
```
Remmeber all files should have headers.

Also this code is for classification not regression and classes should be in numeric format starts from zero.

*Made By Amirhossein Abaskohi*
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#define TRAIN_FILE "train.csv"
#define WEIGHTS_FILE "weights.csv"
#define DELIMITER ','

using namespace std;

class Feature
{
    public:
        void addValue(double val) { values.push_back(val); }
        void normalize();
        void setMax(double featureMax) { max = featureMax; }
        void setMin(double featureMin) { min = featureMin; }
        void calMax();
        void calMin();
        double getValues(int index) { return values[index]; }
        int getSizeOfValues() { return values.size(); }
    private:
        vector<double> values;
        double max;
        double min;
};

void Feature :: normalize()
{  
    for(int i = 0 ; i < values.size() ; i++)
        values[i] = (values[i] - min)/(max - min);
}

void Feature :: calMax()
{
    bool isSet = false;
    double localMax;
    for(int i = 0 ; i < values.size() ; i++)
    {
        if(isSet && localMax < values[i])
            localMax = values[i];
        else if(!isSet)
        {
            localMax = values[i];
            isSet = true;
        }
    }
    max = localMax;
}

void Feature :: calMin()
{
    bool isSet = false;
    double localMin;
    for(int i = 0 ; i < values.size() ; i++)
    {
        if(isSet && localMin > values[i])
            localMin = values[i];
        else if(!isSet)
        {
            localMin = values[i];
            isSet = true;
        }
    }
    min = localMin;
}

void addFeatures(vector<Feature*>& features, string line)
{
    stringstream ss(line);

    while( ss.good() )
    {
        string substr;
        getline( ss, substr, DELIMITER );
        features.push_back(new Feature());
    }
}

void addValues(vector<Feature*>& features, string line)
{
    stringstream ss(line);
    int index = 0;

    while( ss.good() )
    {
        string substr;
        getline( ss, substr, DELIMITER );
        features[index]->addValue(stod(substr));
        index ++;
    }
}

void addWeights(vector<vector<double> >& weights, string line)
{
    stringstream ss(line);
    vector<double> temp;

    while( ss.good() )
    {
        string substr;
        getline( ss, substr, DELIMITER );
        temp.push_back(stod(substr));
    }
    weights.push_back(temp);
}

double calDotMultiplication(vector<double> a, vector<Feature*> features, int index)
{
    double sum = 0;
    vector<double> b;
    for(int i = 0 ; i < features.size()-1 ; i++)
        b.push_back(features[i]->getValues(index));
    for(int i = 0 ; i < a.size()-1 ; i++)
        sum += a[i]*b[i];
    sum += a[a.size()-1];
    return sum;
}

double calAccuracy(vector<vector<double>>& weights, vector<Feature*>& features)
{
    int trueGuesses = 0;
    int numberOfRows = features[0]->getSizeOfValues();
    for(int i = 0 ; i < numberOfRows ; i++)
    {
        bool isSet = false;
        double max;
        double selectedClass;
        for(int j = 0 ; j < weights.size() ; j++)
        {
            double value = calDotMultiplication(weights[j], features, i);
            if(!isSet)
            {
                max = value;
                isSet = true;
                selectedClass = (double)j;
            }
            else if(isSet && max < value)
            {
                max = value;
                selectedClass = j;
            }
        }
        if(selectedClass == features[features.size()-1]->getValues(i))
            trueGuesses ++;
    }
    return ((double)trueGuesses/(double)numberOfRows)*100;
}


int main(int argc, char* argv[])
{
    vector<Feature*> features;
    vector<vector<double> > weights;
    string line;
    int lineNum = 1;
    string datasetDir(argv[1]);
    ifstream trainFile(datasetDir+TRAIN_FILE);
    while(getline(trainFile, line))
    {
        if(lineNum == 1)
            addFeatures(features, line);
        else
            addValues(features, line);
        lineNum ++;
    }
    trainFile.close();
    for(int i = 0 ; i < features.size()-1 ; i++)
    {
        features[i]->calMax();
        features[i]->calMin();
        features[i]->normalize();
    }
    lineNum = 1;
    ifstream weightsFile(datasetDir+WEIGHTS_FILE);
    while(getline(weightsFile, line))
    {
        if(lineNum == 1);
        else
            addWeights(weights, line);
        lineNum++;
    }
    weightsFile.close();
    cout << calAccuracy(weights, features) << "%" << endl;
}
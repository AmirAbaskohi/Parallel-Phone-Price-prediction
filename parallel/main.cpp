#include <iostream>
#include <vector>
#include <string>
#include <pthread.h>
#include <fstream>
#include <sstream>

#define TRAIN_FILE "train_"
#define WEIGHTS_FILE "weights.csv"
#define CSV_EXTENTION ".csv"
#define DELIMITER ','
#define NUMBER_OF_THREADS 4

using namespace std;

class Feature
{
    public:
        void addValue(double val) { values.push_back(val); }
        void normalize();
        void setMax(double featureMax) { max = featureMax; }
        void setMin(double featureMin) { min = featureMin; }
        double getMax() { return max; }
        double getMin() { return min; }
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

pthread_t threads[NUMBER_OF_THREADS];
pthread_mutex_t mutexTrueGuesses;

int numberOfTrueGuesses = 0;
string datasetDir;
vector<vector<Feature*> > features(NUMBER_OF_THREADS);
vector<vector<double> > weights;

void* readTrain(void* arg)
{
    long fileNum = (long)arg;
    string fileName = TRAIN_FILE + to_string(fileNum) + CSV_EXTENTION;
    string line;

    int lineNum = 1;
    ifstream trainFile(datasetDir+fileName);
    while(getline(trainFile, line))
    {
        if(lineNum == 1)
        {
            stringstream ss(line);

            while( ss.good() )
            {
                string substr;
                getline( ss, substr, DELIMITER );
                features[fileNum].push_back(new Feature());
            }
        }
        else
        {
            stringstream ss(line);
            int index = 0;

            while( ss.good() )
            {
                string substr;
                getline( ss, substr, DELIMITER );
                features[fileNum][index]->addValue(stod(substr));
                index ++;
            }
        }
        lineNum ++;
    }
    trainFile.close();
    pthread_exit((void*)0);
}

void* calMaxAndMin(void* arg)
{
    long threadNum = (long)arg;
    for(int i = 0 ; i < features[threadNum].size()-1 ; i++)
    {
        features[threadNum][i]->calMax();
        features[threadNum][i]->calMin();
    }
    pthread_exit((void*)0);
}

void* normalize(void* arg)
{
    long threadNum = (long)arg;
    for(int i = 0 ; i < features[threadNum].size()-1 ; i++)
        features[threadNum][i]->normalize();
    pthread_exit((void*)0);
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

void* predictPriceClass(void* arg)
{
    long threadNum = (long)arg;
    int trueGuesses = 0;
    int numberOfRows = features[threadNum][0]->getSizeOfValues();
    for(int i = 0 ; i < numberOfRows ; i++)
    {
        bool isSet = false;
        double max;
        double selectedClass;
        for(int j = 0 ; j < weights.size() ; j++)
        {
            double value = calDotMultiplication(weights[j], features[threadNum], i);
            if(!isSet)
            {
                max = value;
                isSet = true;
                selectedClass = (double)j;
            }
            else if(isSet && max < value)
            {
                max = value;
                selectedClass = (double)j;
            }
        }
        if(selectedClass == features[threadNum][features[threadNum].size()-1]->getValues(i))
            trueGuesses ++;
    }
    pthread_mutex_lock (&mutexTrueGuesses);
    numberOfTrueGuesses += trueGuesses;
    pthread_mutex_unlock (&mutexTrueGuesses);
	pthread_exit((void*)0);
}

void addWeights(string line)
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

void calAllMaxMin()
{
    int numberOfFeatures = features[0].size() - 1;
    for(int i = 0 ; i < numberOfFeatures ; i++)
    {
        bool isSet = false;
        double allMax;
        double allMin;
        for(int j = 0 ; j < NUMBER_OF_THREADS ; j++)
        {
            if(isSet == false)
            {
                allMax = features[j][i]->getMax();
                allMin = features[j][i]->getMin();
                isSet = true;
            }
            else
            {
                if(features[j][i]->getMax() > allMax)
                    allMax = features[j][i]->getMax();
                if(features[j][i]->getMin() < allMin)
                    allMin = features[j][i]->getMin();
            }
        }
        for(int j = 0 ; j < NUMBER_OF_THREADS ; j++)
        {
            features[j][i]->setMax(allMax);
            features[j][i]->setMin(allMin);
        }
    }
}

int main(int argc, char* argv[])
{
    int lineNum = 1;
    string line;
    string datasetDirTemp(argv[1]);
    datasetDir = datasetDirTemp;

    void* status;

    pthread_mutex_init(&mutexTrueGuesses, NULL);

    for(long i = 0 ; i < NUMBER_OF_THREADS; i++)
        pthread_create(&threads[i], NULL, readTrain, (void*)i);

    ifstream weightsFile(datasetDir+WEIGHTS_FILE);
    while(getline(weightsFile, line))
    {
        if(lineNum == 1);
        else
            addWeights(line);
        lineNum++;
    }
    weightsFile.close();

    for(long i = 0; i < NUMBER_OF_THREADS; i++)
		pthread_join(threads[i], &status);

    for(long i = 0 ; i < NUMBER_OF_THREADS; i++)
        pthread_create(&threads[i], NULL, calMaxAndMin, (void*)i);

    for(long i = 0; i < NUMBER_OF_THREADS; i++)
		pthread_join(threads[i], &status);

    calAllMaxMin();

    for(long i = 0 ; i < NUMBER_OF_THREADS; i++)
        pthread_create(&threads[i], NULL, normalize, (void*)i);

    for(long i = 0; i < NUMBER_OF_THREADS; i++)
		pthread_join(threads[i], &status);

    for(long i = 0 ; i < NUMBER_OF_THREADS; i++)
        pthread_create(&threads[i], NULL, predictPriceClass, (void*)i);

    for(long i = 0; i < NUMBER_OF_THREADS; i++)
		pthread_join(threads[i], &status);

    int numberOfRows = 0;
    for(int i = 0 ; i < NUMBER_OF_THREADS ; i++)
        numberOfRows += features[i][0]->getSizeOfValues();
    cout << ((double)numberOfTrueGuesses/(double)numberOfRows)*100 << "%" << endl;
}
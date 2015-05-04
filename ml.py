from glob import glob
import math
import subprocess

import time
import re
import numpy as np
from sklearn.naive_bayes import MultinomialNB
from sklearn.metrics import *
import heapq

start = time.clock()

def stem(filename):
    return subprocess.check_output(["stemmer.exe", filename], stdin=subprocess.PIPE, shell=True)

def parse_documents():

    traindocs = []
    testdocs = []
    training = [1,2,3]
    testing = [4,5,6]
    testFileNames = []
    for i in training:
        for filename in glob("enron/enron{0}/ham/*.txt".format(i)):
            output = open(filename).read()
            traindocs.append((output,0))
        
        for filename in glob("enron/enron{0}/spam/*.txt".format(i)):
            output = open(filename).read()
            traindocs.append((output,1))

    for i in testing:
        for filename in glob("enron/enron{0}/ham/*.txt".format(i)):
            output = open(filename).read()
            testdocs.append((output,0))
            testFileNames.append(filename)
        
        for filename in glob("enron/enron{0}/spam/*.txt".format(i)):
            output = open(filename).read()
            testdocs.append((output,1))
            testFileNames.append(filename)

            
                
    return (traindocs,testdocs,testFileNames)
# 0 for ham, 1 for spam
traindocs,testdocs,testFileNames = parse_documents()

def get_stopwords():
    lines = [line.strip().lower() for line in open('stopwords.txt')]
    return set(lines)
stopwords =  get_stopwords()

def get_features(docs):
    global stopwords
    bannedCharacters = set('!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~0123456789')
    words = {}
    for text, isSpam in docs:
        if text != "":
            docwords = ''.join(c for c in text.lower() if c not in bannedCharacters).split()
            for w in docwords:
                if w not in stopwords:
                    if w not in words:
                        words[w] = 1
                    else:
                        words[w] += 1
    for w in words.keys():
        if words[w] < 100:
            words.pop(w,None)
    wordsList = words.keys()
    mapWords = {}
    for i in range(len(wordsList)):
        mapWords[wordsList[i]] = i
    return (wordsList,mapWords)
features,mapFeatures = get_features(traindocs)

def percentCorrect(true, predicted):
    count = 0.0
    for i in range(len(true)):
        if true[i] == predicted[i]:
            count+=1
    return count/len(true)

def get_datapoints(docs,features,mapFeatures):
    bannedCharacters = set('!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~0123456789')
    dataX = np.zeros([len(docs),len(features)])
    dataY = np.zeros(len(docs))
    dataW = [None] * len(docs)
    for i in range(len(docs)):
        doc = docs[i]
        words = set()
        if doc[0] != "":
            docwords = ''.join(c for c in doc[0].lower() if c not in bannedCharacters).split()
            for w in docwords:
                if w in mapFeatures:
                    word = mapFeatures[w]
                    words.add(word)
                    dataX[i][word] += 1
        dataY[i] = doc[1]
        dataW[i] = words
    return (dataX,dataY,dataW)

trainX,trainY,trainW = get_datapoints(traindocs,features,mapFeatures)
testX,testY,testW = get_datapoints(testdocs,features,mapFeatures)

def normalizeIDF(trainX,testX,trainW,features):
    idf = np.zeros(len(features))
    for wordList in trainW:
        for word in wordList:
            idf[word] += 1
    #for i in range(len(trainX)):
    #    for j in range(len(features)):
    #        if trainX[i][j] > 0:
    #            idf[j] += 1
    idf = np.log(len(trainX)/idf)
    #trainX1 = np.zeros([len(trainX),len(trainX[0])])
    #testX1 = np.zeros([len(testX),len(testX[0])])
    for i in range(len(trainX)):
        trainX[i] = trainX[i]*idf
        if np.linalg.norm(trainX[i]) > 0:
            trainX[i] = trainX[i]/np.linalg.norm(trainX[i])
    for i in range(len(testX)):
        testX[i] = testX[i]*idf
        if np.linalg.norm(testX[i]) > 0:
            testX[i] = testX[i]/np.linalg.norm(testX[i])
    return
normalizeIDF(trainX,testX,trainW,features)
del trainW
del testW
print "Finished Loading Data: ", time.clock()- start, "secs"

#store in file
def storeFeatures(testX, testY):
    import cPickle as pk
    pk.dump(testX, open('testX.pk','w'))
    pk.dump(testY, open('testY.pk','w'))

def loadFeatures():
    import cPickle as pk
    return map(pk.load, (open('testX.pk'), open('testY.pk')))
    
#storeFeatures(testX,testY)
#(testX,testY) = loadFeatures()
#print "loaded"

def trainNB(trainX,trainY,testX,testY,samples,limit):
    start = time.clock()
    clf = MultinomialNB()
    clf.fit(trainX[:samples], trainY[:samples])
    print time.clock()-start
    start = time.clock()
    
    predicted = clf.predict(trainX[0:samples])
    print "percent Trained correct: ", percentCorrect(trainY[:samples],predicted)
    print "f-score: ", f1_score(trainY[:samples],predicted)
    metric = precision_recall_fscore_support(trainY[:samples],predicted)
    print "precision: ", metric[0]
    print "recall: ", metric[1]

    predicted = clf.predict(testX[0:limit])
    print "percent Test correct: ", percentCorrect(testY[:limit],predicted)
    print "f-score: ", f1_score(testY[:limit],predicted)
    metric = precision_recall_fscore_support(testY[:limit],predicted)
    print "precision: ", metric[0]
    print "recall: ", metric[1]

    print time.clock()-start
    return clf
clf = trainNB(trainX,trainY,testX,testY,-1,-1)
logProb = clf.feature_log_prob_
classProb = clf.class_log_prior_

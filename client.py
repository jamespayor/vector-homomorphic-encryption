from model import features
import numpy as np
from hevector import evaluate

def get_stopwords():
    lines = [line.strip().lower() for line in open('stopwords.txt')]
    return set(lines)
stopwords =  get_stopwords()

def get_feature_vector(doc):
    global stopwords
    global features
    bannedCharacters = set('!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~0123456789')
    dataX = np.zeros(len(features))
    dataX[0] = 1
    if doc != "":
        docwords = ''.join(c for c in doc.lower() if c not in bannedCharacters).split()
        for w in docwords:
            if w in features:
                dataX[features[w]] += 1
    return dataX

text = "This is spam"

vector = tuple(map(int,get_feature_vector(text)))

vector = (1,2,3)
C,T,SK = evaluate([vector, 'random-matrix', len(vector), 'duplicate-matrix',
          'encrypt', 'duplicate-matrix', 'get-secret-key'])
print C

PK, = evaluate(['identity', len(vector), T, 'key-switch-matrix'])

#keySwitchM = evaluate([T,'inner-product-key-switch'])
#print keySwitchM

#print C, T, SK, PK, keySwitchM
print evaluate([C, SK, 'decrypt'])[0]

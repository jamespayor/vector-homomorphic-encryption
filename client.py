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

n = len(features)
split = 10
feature_vector = get_feature_vector(text)
feature_vector_splits = []
secretKeysCommands = []
for i in range(n/split+1):
    feature_vector_splits.append(feature_vector[i*split:max((i+1)*split,n-1)])
    secretKeysCommands.append('random-matrix')
    secretKeysCommands.append(split)

secretKeys = evaluate(secretKeysCommands)

encryptCommands = []
for i in range(n/split+1):
    encryptCommands.append(feature_vector_splits[i])
    encryptCommands.append(secretKeys[i])
    encryptCommands.append('get-secret-key')
    encryptCommands.append('encrypt')

ciphertexts = evaluate(encryptCommands)

encryptedResults = getEncryptedResults(ciphertexts)

decryptCommands = []
for i in range(n/split+1):
    decryptCommands.append(encryptedResults[i])
    decryptCommands.append(secretKeys[i])
    decryptCommands.append('inner-product-no-switch-decrypt')

decryptedResults = evaluate(decryptCommands)
result = 0.0
for i in range(n/split+1):
    result+=decryptedResults[i]

print result
#vector = tuple(map(int,))[:100]
##
##vector = (1,2,3)
##C,T,SK = evaluate([vector, 'random-matrix', len(vector), 'duplicate-matrix','encrypt', 'duplicate-matrix', 'get-secret-key'])
##print C
##
###keySwitchM = evaluate([T,'inner-product-key-switch'])
##
###print C, T, SK, PK, keySwitchM
##result, = evaluate([C, SK, 'decrypt'])
##print result

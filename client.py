import numpy as np
from hevector import evaluate

def get_features():
	from model import features
	return features

def get_split():
	from model import split
	return split


def get_feature_vector(doc):
	features = get_features()
	bannedCharacters = set('!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~0123456789')
	dataX = np.zeros(len(features))
	dataX[0] = 1
	if doc != "":
		docwords = ''.join(c for c in doc.lower() if c not in bannedCharacters).split()
		for w in docwords:
			if w in features:
				dataX[features[w]] += 1
	return tuple(map(int,dataX))


def inf(x):
	while True:
		yield x

def flatten(x):
	for y in x:
		for z in y:
			yield z

def flatzip(*args):
	return list(flatten(zip(*args)))

def getSecretKey():
	#import cPickle as pk
	#return pk.load(open('secretKey.pk'))
	return 'vector ' + open('secretKey.txt').read()

def searchResults(text):

	featureVector = get_feature_vector(text)
	weightMatrix = (tuple(featureVector),)

	print "Loading secret key..."
	secretKey = getSecretKey();
	
	T, S = evaluate(['random-matrix', 1, 1, 'duplicate-matrix', 'get-secret-key'])
	keySwitch = evaluate([weightMatrix, secretKey, T, 'linear-transform-key-switch'])

	print "HIII!"
	encryptedResults = getLinearTransformations(keySwitch)

	results = evaluate(flatzip(inf(S), results, inf('decrypt')))

	return results


def getLinearTransformations(keySwitch):
	import requests as rq
	print rq.post('http://127.0.0.1:8000/search', data={'keySwitch': str(keySwitch)}).text
	raise Exception



from sys import stdin
text = stdin.read()

print '\n'.join(map(str,searchResults(text)))



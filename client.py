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
	return 'matrix ' + open('secretKey.txt').read()

def searchResults(text):

	featureVector = get_feature_vector(text)
	weightMatrix = (tuple(featureVector),)

	print "Loading secret key..."
	secretKey = getSecretKey();
	
	print "Getting key switch matrix..."
	T, S = evaluate(['random-matrix', 1, 1, 'duplicate-matrix', 'get-secret-key'])
	print "(Computing it now...)"
	keySwitch, = evaluate([weightMatrix, secretKey, T, 'linear-transform-key-switch'])

	print "Getting encrypted results..."
	encryptedResults = getLinearTransformations(keySwitch)

	print "Decrypting results..."
	results = evaluate([S] + flatzip(inf('duplicate-matrix'), encryptedResults, inf('decrypt')))[:-1]

	return (r[0] for r in results)


def getLinearTransformations(keySwitch):
	import requests as rq
	global post
	# 18.111.110.103
	# 0.0.0.0
	success = False
	while not success:
		try:
			post = rq.post('http://18.111.110.103:8000/search', data={'keySwitch': repr(keySwitch)})
			success = True
		except:
			print "Failed.  Trying again!"
	
	print "Result: ", repr(post.text)
	return tuple(tuple(int(y.strip().strip('L')) for y in x.strip('(').strip(')').split(',')) for x in post.text.splitlines())


from sys import stdin
text = stdin.read()

print '\n'.join(map(str,searchResults(text)))



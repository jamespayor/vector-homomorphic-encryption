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

def classify(text):

	feature_vector = get_feature_vector(text)

	feature_vector_splits = []
	split = get_split()
	index = 0
	while index < len(feature_vector):
		feature_vector_splits.append(feature_vector[index:index+split])
		index += split

	print "Getting secret keys..."
	secretKeysCommands = flatzip(inf('random-matrix'), map(len, feature_vector_splits), inf('duplicate-matrix'), inf('get-secret-key'))
	secretKeysAndTs = evaluate(secretKeysCommands)
	secretKeyTs = secretKeysAndTs[::2]
	secretKeys = secretKeysAndTs[1::2]

	print "Getting ciphertexts..."
	encryptCommands = flatzip(feature_vector_splits, secretKeyTs, inf('encrypt'))
	ciphertexts = evaluate(encryptCommands)

	print "Getting inner products..."
	encryptedResults = getInnerProducts(ciphertexts)

	classification = dict()
	for name, results in encryptedResults:
		print "Decrypting inner products for '%s'..." % name
		decryptCommands = flatzip(results, secretKeys, inf('inner-product-no-switch-decrypt'))
		decryptedResults = evaluate(decryptCommands)
		#print decryptedResults
		classification[name] = sum(flatten(decryptedResults))

	return classification

def getInnerProducts(ciphertexts):
	from server import get_inner_products
	return get_inner_products(ciphertexts)



from sys import stdin
text = stdin.read()

print '\n'.join(map(str,classify(text).items()))



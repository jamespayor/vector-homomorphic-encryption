from hevector import evaluate
from cgi import escape as htmlEscape
import cPickle as pk
fileNames = pk.load(open('testFiles.pk'))

# NOTE: Set this to the location of the server, and the port it's listening on.
SERVER_LOCATION = "127.0.0.1"
SERVER_PORT = 80
# If you're running it locally, just update the port. Otherwise, use an IP address or a a domain name for the server location.

def get_features():
	from model import features
	return features

def get_split():
	from model import split
	return split


def simplifyText(text):
	bannedCharacters = set('!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~0123456789')
	features = get_features()
	return ' '.join(w for w in ''.join(c for c in text.lower() if c not in bannedCharacters).split() if w in features).strip()

def get_feature_vector(text):
	features = get_features()
	dataX = [0] * len(features)
	dataX[0] = 1
	for w in simplifyText(text).split():
		dataX[features[w]] += 1
	return dataX


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

def getSearchWeightMatrix(text):
	featureVector = get_feature_vector(text)
	weightMatrix = (tuple(featureVector),)
	return weightMatrix

def getSpamWeightMatrix():
	from model import weightings
	return (weightings['ham'], weightings['spam'])


def getResults(weightMatrix):

	print "Client: Loading secret key..."
	secretKey = getSecretKey();
	
	print "Client: Getting new secret key..."
	T, S = evaluate(['random-matrix', len(weightMatrix), 1, 'duplicate-matrix', 'get-secret-key'])
	print "Client: Getting key switch matrix..."
	keySwitch, = evaluate([weightMatrix, secretKey, T, 'linear-transform-key-switch'])

	print "Client: Getting encrypted results from server..."
	encryptedResults = getLinearTransformations(keySwitch)

	print "Client: Decrypting results..."
	results = evaluate([S] + flatzip(inf('duplicate-matrix'), encryptedResults, inf('decrypt')))[:-1]

	print "Client: Done."
	print
	print
	return results


def getLinearTransformations(keySwitch):
	import requests as rq
	post = rq.post('http://%s:%s/transform' % (SERVER_LOCATION, SERVER_PORT), data={'keySwitch': repr(keySwitch)})
	return tuple(tuple(int(y.strip().strip('L')) for y in x.strip('(').strip(')').split(',')) for x in post.text.splitlines())


def getSearchResults(text):
	print getResults(getSearchWeightMatrix(text))
	return tuple(i for i,x in sorted(enumerate(getResults(getSearchWeightMatrix(text))), key=lambda (i,x): -x[0]) if x[0] > 1)

def getSpamResults():
	return tuple((i, (ham > spam)) for i, (ham, spam) in enumerate(getResults(getSpamWeightMatrix())))

def getFileDescription(i):
	name = fileNames[i]
	subject = open(name).readline()
	return '%s <i>(%s)</i>' % (htmlEscape(subject), htmlEscape(name.split('/')[-1]))

def getLink(i):
	return "/view/%d" % i

def getFormattedSearchResults(text):
	return tuple('<p style="font-size: large;"><a href="%s">%s</a></p>' % (getLink(i), getFileDescription(i)) for i in getSearchResults(text))

def getFormattedSpamResults():
	return tuple('<p style="font-size: large;"><a href="%s">%s %s</a></p>' % (getLink(i), "" if ham else "[SPAM]", getFileDescription(i)) for i, ham in getSpamResults())



from flask import Flask, request, render_template
app = Flask(__name__)


@app.route('/')
def index():
	return render_template('index.html')

@app.route('/search', methods=['GET', 'POST'])
def search():
	text = request.form.get('text', None)
	results = None
	noResults = None
	if text:
		text = simplifyText(text)
		if text:
			results = getFormattedSearchResults(text)
		if not results:
			noResults = True
	return render_template('search.html', text=text, results=results, noResults=noResults)

@app.route('/spam', methods=['GET'])
def spam():
	try:
		results = getFormattedSpamResults()
	except Exception as e:
		print e
		results = None
	return render_template('spam.html', results=results)

@app.route('/view/<int:i>')
def view(i):
	name = fileNames[i]
	lines = list(open(name).read().decode('utf8','ignore').replace(' | ', 'l').splitlines())
	subject = lines[0]
	text = '\n'.join(lines[1:]).replace('.\r\n','.\n\n')
	return render_template('view.html', name=name, subject=subject, text=text)


if __name__ == "__main__":
	app.run(host='0.0.0.0', port=6857, debug=False)


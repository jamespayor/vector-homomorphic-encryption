from flask import Flask, request
app = Flask(__name__)

from model import features, probabilities

@app.route('/')
def route_index():
    return '''
    	<html>
    		<body>
    			<h1> Homomorphic Email Classification Service! </h1>
    			<p> This server gives likelihoods that an email is spam or ham given an encrypted form of the original email,
    			without gaining access to any information about the message. </p>
    			<p> Get the features description from '/features' - each feature being a linear combination of word counts
    			in the document. </p>
    			<p> Submit requests by POST with a 'features' tuple of vectors (tuples) to '/classify'. </p>
    		</body>
    	</html>
    	'''

@app.route('/features', methods=['GET'])
def route_features():
	return repr(features)

@app.route('/classify', methods=['GET', 'POST'])
def route_classify():
	features = tuple(map(int,(x.strip().strip('(') for x in t.strip().strip('(').strip(')').split(',') if x.strip().strip('('))) for t in request.args['features'].split(')') if t.strip())
	results = get_inner_products(features)
	return '\n'.join('%s %r' % result for result in results)

def get_inner_products(features):
	from model import weightings
	results = list()
	for name, weighting in weightings.items():
		weights = list()
		index = 0
		for l in map(len, features):
			weights.append(weighting[index:index+l])
			index += l
		from hevector import evaluate
		res = evaluate([x for y in zip(weights, features, ('inner-product-no-switch' for i in range(len(weights)))) for x in y])
		results.append((name, res))
	return results

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000, debug=True)

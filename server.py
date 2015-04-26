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
    			<p> Submit requests by POST with a 'features' tuple and a 'public_key' tuple of tuples to '/classify'. </p>
    		</body>
    	</html>
    	'''

@app.route('/features', methods=['GET'])
def route_features():
	return repr(features)

@app.route('/classify', methods=['GET', 'POST'])
def route_classify():
	return 'TODO: Classification.  What you sent us: %r, %r' % (request.args['features'], request.args['public_key'])

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000, debug=True)

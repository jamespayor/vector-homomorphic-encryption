import model

def get_stopwords():
    lines = [line.strip().lower() for line in open('stopwords.txt')]
    return set(lines)
stopwords =  get_stopwords()

def get_feature_vector(doc, mapFeatures):
    global stopwords
    bannedCharacters = set('!"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~0123456789')
    dataX = np.zeros(len(mapFeatures))
    dataX[0] = 1
    if doc != "":
        docwords = ''.join(c for c in doc.lower() if c not in bannedCharacters).split()
        for w in docwords:
            if w in mapFeatures:
                dataX[mapFeatures[w]] += 1
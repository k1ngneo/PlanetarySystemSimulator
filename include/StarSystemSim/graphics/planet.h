#include "StarSystemSim/graphics/model.h"

namespace graphics {

	class Planet : public Model {
	public:
		Planet(const char* name);
		~Planet();

	private:
		void subdivide(int depth = 1);
		void divideTriangle(std::vector<Mesh::VertexData>& nVertices, std::vector<GLuint>& nIndices, int ind1, int ind2, int ind3);
		void calcUVs(std::vector<Mesh::VertexData>& vertices);
		void calcNormals(std::vector<Mesh::VertexData>& vertices);

		GLuint lookUpOrAdd(std::vector<Mesh::VertexData>& nVertices, std::pair<int, int> pair);
	};

}
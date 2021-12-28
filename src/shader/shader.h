#ifndef shader_hpp
#define shader_hpp

#include <string>
#include <vector>

class Shader
{
public:
	Shader();
	void init(const std::string& vertex_code, const std::string& fragment_code);
	void use();
	template<typename T> void setUniform(const std::string& name, T val);
	template<typename T> void setUniform(const std::string& name, T val1, T val2);
	template<typename T> void setUniform(const std::string& name, T val1, T val2, T val3);

private:
	void checkCompileErr();
	void checkLinkingErr();
	void compile();
	void link();
	unsigned int vertex_id_, fragment_id_, id_;
	std::string vertex_code_;
	std::string fragment_code_;
};

#endif /* shader_hpp */

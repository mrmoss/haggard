/**
  UAV control, implementation of network JSON conversion functions.
  
  Dr. Orion Lawlor, lawlor@alaska.edu, 2014-04-28 (Public Domain)
*/
#include <stdexcept>
#include "cyberalaska/uav_control_JSON.h"
#include "json/json.h"



Json::Value parse_JSON(const std::string &jsonString)
{
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(jsonString,root)) throw std::runtime_error("Error parsing JSON: "+reader.getFormattedErrorMessages());
	return root;
}


void from_JSON(Json::Value &node,std::string &v) {
	v=node.asString();
}
void from_JSON(Json::Value &node,float &v) {
	v=node.asFloat();
}
void from_JSON(Json::Value &node,const char *name,float &v) {
	v=node.get(name,-999).asFloat();
}

void from_JSON(Json::Value &node,vec2 &v) {
	from_JSON(node,"x",v.x);
	from_JSON(node,"y",v.y);
}

template <class T>
void from_JSON(Json::Value &node,std::vector<T> &v) {
	if (!node.isArray()) return;
	v.resize(node.size());
	for (int i=0;i<node.size();i++)
		from_JSON(node[i],v[i]);
}

void from_JSON(Json::Value &root,AK_uav_field &v) {
	from_JSON(root["state"],v.state); 
	from_JSON(root["uav"],v.uav);
	from_JSON(root["obstacles"],v.obstacles);
	from_JSON(root["hikers"],v.hikers);
}

AK_uav_field AK_uav_field_from_JSON(const std::string &jsonString)
{
	AK_uav_field out;
	Json::Value root=parse_JSON(jsonString);
	from_JSON(root,out);
	return out;
}

void from_JSON(Json::Value &root,AK_uav_control_sensors &uav) {
	from_JSON(root["state"],uav.state); 
	from_JSON(root["x"],uav.x); 
	from_JSON(root["y"],uav.y);
	for (int dir=0;dir<n_directions;dir++) {
		from_JSON(root["obstacle"][dir],uav.obstacle[dir]);
		from_JSON(root["hiker"][dir],uav.hiker[dir]);
	}
}
AK_uav_control_sensors  AK_uav_control_sensors_from_JSON (const std::string &jsonString)
{
	AK_uav_control_sensors uav;
	Json::Value root=parse_JSON(jsonString);
	from_JSON(root,uav);
	return uav;
}

/**** JSON output *****/

Json::Value make_JSON(const std::string &v) {
	return v;
}
Json::Value make_JSON(const vec2 &v) {
	Json::Value ret;
	ret["x"]=v.x; ret["y"]=v.y;
	return ret;
}
template <class T>
Json::Value make_JSON(const std::vector<T> &v) {
	Json::Value ret;
	for (unsigned int i=0;i<v.size();i++) {
		ret[i]=make_JSON(v[i]);
	}
	return ret;
}

Json::Value make_JSON(const AK_uav_field &out) {
	Json::Value ret;
	ret["state"]=make_JSON(out.state);
	ret["uav"]=make_JSON(out.uav);
	ret["obstacles"]=make_JSON(out.obstacles);
	ret["hikers"]=make_JSON(out.hikers);
	return ret;
}

std::string JSON_from_AK_uav_field(const AK_uav_field &out)
{
	Json::Value root=make_JSON(out);
	return Json::FastWriter().write(root);
}

std::string JSON_from_AK_uav_control_sensors (const AK_uav_control_sensors &uav)
{
	Json::Value root;
	root["state"]=uav.state;
	root["x"]=uav.x; root["y"]=uav.y;
	for (int dir=0;dir<n_directions;dir++) {
		root["obstacle"][dir]=uav.obstacle[dir];
		root["hiker"][dir]=uav.hiker[dir];
	}
	return Json::FastWriter().write(root);
}






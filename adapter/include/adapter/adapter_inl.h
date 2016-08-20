namespace adapter
{

xml_node_browse_name const &xml_node_browse_path::last() const
{
	return elements.back();
}

xml_node_browse_name::operator std::string() const
{
	return str();
}

xml_node_browse_path::operator std::string() const
{
	return str();
}

}

#include <bayesian_webclass/http_downloader.h>
#include <libxml++/libxml++.h>
#include <cstdlib>
#include <boost/filesystem.hpp>



void print_indentation(unsigned int indentation)
{
  for(unsigned int i = 0; i < indentation; ++i)
    std::cout << " ";
}

void print_node(const xmlpp::Node* node, unsigned int indentation = 0)
{
  std::cout << std::endl; //Separate nodes by an empty line.
  
  const xmlpp::ContentNode* nodeContent = dynamic_cast<const xmlpp::ContentNode*>(node);
  const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(node);
  const xmlpp::CommentNode* nodeComment = dynamic_cast<const xmlpp::CommentNode*>(node);

  if(nodeText && nodeText->is_white_space()) //Let's ignore the indenting - you don't always want to do this.
    return;
    
  Glib::ustring nodename = node->get_name();

  if(!nodeText && !nodeComment && !nodename.empty()) //Let's not say "name: text".
  {
    print_indentation(indentation);
    std::cout << "Node name = " << node->get_name() << std::endl;
    std::cout << "Node name = " << nodename << std::endl;
  }
  else if(nodeText) //Let's say when it's text. - e.g. let's say what that white space is.
  {
    print_indentation(indentation);
    std::cout << "Text Node" << std::endl;
  }

  //Treat the various node types differently: 
  if(nodeText)
  {
    print_indentation(indentation);
    std::cout << "text = \"" << nodeText->get_content() << "\"" << std::endl;
  }
  else if(nodeComment)
  {
    print_indentation(indentation);
    std::cout << "comment = " << nodeComment->get_content() << std::endl;
  }
  else if(nodeContent)
  {
    print_indentation(indentation);
    std::cout << "content = " << nodeContent->get_content() << std::endl;
  }
  else if(const xmlpp::Element* nodeElement = dynamic_cast<const xmlpp::Element*>(node))
  {
    //A normal Element node:

    //line() works only for ElementNodes.
    print_indentation(indentation);
    std::cout << "     line = " << node->get_line() << std::endl;

    //Print attributes:
    const xmlpp::Element::AttributeList& attributes = nodeElement->get_attributes();
    for(xmlpp::Element::AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
    {
      const xmlpp::Attribute* attribute = *iter;
      print_indentation(indentation);
      std::cout << "  Attribute " << attribute->get_name() << " = " << attribute->get_value() << std::endl;
    }

    const xmlpp::Attribute* attribute = nodeElement->get_attribute("title");
    if(attribute)
    {
      std::cout << "title found: =" << attribute->get_value() << std::endl;
      
    }
  }
  
  if(!nodeContent)
  {
    //Recurse through child nodes:
    xmlpp::Node::NodeList list = node->get_children();
    for(xmlpp::Node::NodeList::iterator iter = list.begin(); iter != list.end(); ++iter)
    {
      print_node(*iter, indentation + 2); //recursive
    }
  }
}



int main(){
	HTTPDownloader downloader;

    //download html text from html_address
    //std::string content = downloader.download(html_address);
    std::vector<std::string> addresses = downloader.get_urls_from_file("html/http_addresses.txt");
  	xmlpp::DomParser parser;
    std::string html_text,filename;
    std::string path_root("xml/");
    int count = 1;

    boost::filesystem::create_directories ("xml");
 	
    for (std::string i : addresses)
    {
    	html_text = downloader.download(i); //for every link from file download the html code
    	filename = path_root + "kod_html" + std::to_string(count) + ".txt";
    	downloader.write_str_to_file(filename, html_text);
    	html_text = downloader.cleanhtml(html_text); //clean downloaded html code
    	filename = path_root + "kod_html" + std::to_string(count) + "_clean.txt";
    	downloader.write_str_to_file(filename, html_text);
    	parser.parse_memory(html_text);	 //parse html code
    	xmlpp::Node* rootNode = parser.get_document()->get_root_node();

        if (parser)
        { 
            //Walk the tree
            const xmlpp::Node* pNode = parser.get_document()->get_root_node();
            print_node(pNode);
        }
        // const xmlpp::TextNode* nodeText = dynamic_cast<const xmlpp::TextNode*>(rootNode);
    	// Xpath query
    	//xmlpp::NodeSet result = rootNode->find("/html/head/title");
        // std::cout << nodeText->get_content() <<std::endl;
   //  	// Get first node from result



     	//xmlpp::Node *firstNodeInResult = result.at(0);
   //  	// Cast to Attribute node (dynamic_cast on reference can throw [fail fast])
   //  	xmlpp::Attribute &attribute = dynamic_cast<xmlpp::Attribute&>(*firstNodeInResult);

   //  	// Get value of the attribute
   //  	Glib::ustring attributeValue = attribute.get_value();
      	count++;
   //  	// Print attribute value
 		// std::cout << attributeValue << std::endl;
    }

}

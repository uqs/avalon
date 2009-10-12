//
// C++ Implementation: CurlTester
//
// Description:
//
//
// Author: David Frey, ETZ G71.1 <freyd@ee.ethz.ch>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "CurlTester.h"
#include "Curl.h"
#include "Exception.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

using namespace std;

CurlTester::CurlTester()
{
}


CurlTester::~CurlTester()
{
}


bool CurlTester::test()
{
	bool success = true, s;
	cout << "what do you wanna test?" << endl;
	vector<string> choices;
	choices.push_back("fetch an URL");
	choices.push_back("put a string to an URL");
	choices.push_back("put a file to an URL");
	choices.push_back("go back");
	
	do {
		unsigned int c = ask(choices);
		s = false;
		try {
			switch(c) {
				case 0:
					s = test_fetch();
					break;
				case 1:
					s = test_put_string();
					break;
				case 2:
					s = test_put_file();
					break;
				case 3:
					return success;
				default:
					continue;
			}
		}
		catch(Exception& e) {
			cout << "error: " << e.what() << endl;
		}
		catch(...) {
			cout << "unknown error occurred" << endl;
		}
		if( s ) {
			cout << "all test passed" << endl;
		}
		else {
			cout << "some tests failed" << endl;
		}
		success &= s;
	} while(1);
	return success;
}


/*!
    \fn CurlTester::test_fetch
 */
bool CurlTester::test_fetch()
{
	string url;
	cout << "enter an url to fetch: " << flush;
	getline(cin, url);
	
	Curl curl;
	std::vector<char> content = curl.fetch(url);
	cout << endl << "the content of the url is:" << endl;
	cout << std::string(content.begin(), content.end());
	cout << endl << "end of content" << endl;
	return true;
}


/*!
    \fn CurlTester::test_put_string()
 */
bool CurlTester::test_put_string()
{
	string url, str;
	cout << "enter an url to put to: " << flush;
	getline(cin, url);

	cout << "enter a string to send: " << flush;
	getline(cin, str);
	Curl curl;
	vector<char> data;
	data.insert(data.begin(), str.begin(), str.end());
	curl.put(url, data);
	return true;
}


/*!
    \fn CurlTester::test_put_file()
 */
bool CurlTester::test_put_file()
{
	string url, filename;
	cout << "enter an url to put to: " << flush;
	getline(cin, url);
	
	cout << "enter the name of the file to send: " << flush;
	getline(cin, filename);
	Curl curl;
	ifstream file(filename.c_str());
	curl.put(url, &file);
	return true;
}

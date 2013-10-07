/*
 * HtmlGuiComponent.cpp
 *
 *  Created on: 2011-05-08
 *      Author: jarrett
 */

#include <iostream>
#include <string.h>
#include <cstring>

#include <boost/log/trivial.hpp>

#include "HtmlGuiComponent.h"

#include "GUIObject.h"

#include "../common/utilities/ImageLoader.h"

#define DEBUG_PAINT true

namespace glr {
namespace gui {
HtmlGuiComponent::HtmlGuiComponent(glw::IOpenGlDevice* openGlDevice, glmd::uint32 width, glmd::uint32 height) : openGlDevice_(openGlDevice), width_(width), height_(height)
{
	isVisible_ = false;
}

HtmlGuiComponent::~HtmlGuiComponent()
{
	unload();
}

int HtmlGuiComponent::load()
{
	// Create texture to hold rendered view
	glGenTextures(1, &web_texture);
	glBindTexture(GL_TEXTURE_2D, web_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	CefMainArgs args(0, nullptr);
	BOOST_LOG_TRIVIAL(info) << "here 3";

    
        int result1 = CefExecuteProcess(args, nullptr);
        // checkout CefApp, derive it and set it as second parameter, for more control on
        // command args and resources.
        if (result1 >= 0) // child proccess has endend, so exit.
        {
			BOOST_LOG_TRIVIAL(info) << "RESULT 1";
            return result1;
        }
        if (result1 == -1)
        {
			BOOST_LOG_TRIVIAL(info) << "RESULT 2 ";
            // we are here in the father proccess.
        }
    
BOOST_LOG_TRIVIAL(info) << "here 4";
	
        CefSettings settings;

        // checkout detailed settings options http://magpcss.org/ceforum/apidocs/projects/%28default%29/_cef_settings_t.html
        // nearly all the settings can be set via args too.
        // settings.multi_threaded_message_loop = true; // not supported, except windows
        // CefString(&settings.browser_subprocess_path).FromASCII("sub_proccess path, by default uses and starts this executeable as child");
        // CefString(&settings.cache_path).FromASCII("");
        // CefString(&settings.log_file).FromASCII("");
        // settings.log_severity = LOGSEVERITY_DEFAULT;
        // CefString(&settings.resources_dir_path).FromASCII("");
        // CefString(&settings.locales_dir_path).FromASCII("");

        bool result = CefInitialize(args, settings, nullptr);
        // CefInitialize creates a sub-proccess and executes the same executeable, as calling CefInitialize, if not set different in settings.browser_subprocess_path
        // if you create an extra program just for the childproccess you only have to call CefExecuteProcess(...) in it.
        if (!result)
        {
            // handle error
            BOOST_LOG_TRIVIAL(error) << "Error loading HtmlGuiComponent - could not initialize CEF";
            return -1;
        }
    
    BOOST_LOG_TRIVIAL(info) << "here 5";
    // create browser-window
    CefRefPtr<CefBrowser> browser;
    CefRefPtr<BrowserClient> browserClient;
    BOOST_LOG_TRIVIAL(info) << "here 6";
    {
        CefWindowInfo window_info;
        CefBrowserSettings browserSettings;
BOOST_LOG_TRIVIAL(info) << "here 7";
        // in linux set a gtk widget, in windows a hwnd. If not available set nullptr - may cause some render errors, in context-menu and plugins.
        window_info.SetAsOffScreen(nullptr);
		BOOST_LOG_TRIVIAL(info) << "here 8";
		
		RenderHandler* rh = new RenderHandler();
		BOOST_LOG_TRIVIAL(info) << "here 9";
        browserClient = new BrowserClient(rh);
        BOOST_LOG_TRIVIAL(info) << "here 10";

        browser = CefBrowserHost::CreateBrowserSync(window_info, browserClient.get(), "http://deanm.github.io/pre3d/monster.html", browserSettings);
        BOOST_LOG_TRIVIAL(info) << "here 11";

        // inject user-input by calling
        // browser->GetHost()->SendKeyEvent(...);
        // browser->GetHost()->SendMouseMoveEvent(...);
        // browser->GetHost()->SendMouseClickEvent(...);
        // browser->GetHost()->SendMouseWheelEvent(...);
    }

	testint = 31;
	webTextureReady_ = false;
	needs_full_refresh = true;
	
	glw::GlError err = openGlDevice_->getGlError();
	if (err.type != GL_NONE)
	{
		// TODO: throw error
		BOOST_LOG_TRIVIAL(error) << "Error loading HtmlGuiComponent in opengl";
		BOOST_LOG_TRIVIAL(error) << "OpenGL error: " << err.name;
	}
	else
	{
		BOOST_LOG_TRIVIAL(debug) << "Successfully loaded HtmlGuiComponent.";
	}

	return 0;
}

void HtmlGuiComponent::unload()
{
	browser_ = nullptr;
    browserClient_ = nullptr;
	CefShutdown();
        
	this->setVisible(false);
	//window_ = 0;
}

/** Maps an input coordinate to a texture coordinate for injection into
 *  Berkelium.
 *  \param glut_size the size of the GLUT window
 *  \param glut_coord the coordinate value received from GLUT
 *  \param tex_size the size of the texture/Berkelium window
 *  \returns the coordinate transformed to the correct value for the texture /
 *           Berkelium window
 */
unsigned int HtmlGuiComponent::mapGLUTCoordToTexCoord(unsigned int glut_coord, unsigned int glut_size, unsigned int tex_size)
{
    return (glut_coord * tex_size) / glut_size;
}

void HtmlGuiComponent::mouseMoved(glm::detail::int32 xPos, glm::detail::int32 yPos)
{
	//std::cout << "MOUSE MOVED EVENT: " << xPos << " " << yPos << std::endl;
	
	unsigned int tex_coord_x = mapGLUTCoordToTexCoord(xPos, width_, width_);
    unsigned int tex_coord_y = mapGLUTCoordToTexCoord(yPos, height_, height_);
    //std::cout << xPos << " " << yPos << " : " << tex_coord_x << " " << tex_coord_y << std::endl;
	//window_->mouseMoved(tex_coord_x, tex_coord_y);
}

void HtmlGuiComponent::mouseButton(glm::detail::uint32 buttonID, bool down, glm::detail::int32 clickCount)
{
	//std::cout << "MOUSE BUTTON EVENT: " << buttonID << " " << down << std::endl;
	//window_->mouseButton(buttonID, down, clickCount);
}

void HtmlGuiComponent::mouseClick(glm::detail::uint32 buttonID)
{
	//window_->mouseButton(buttonID, true, 1);
}

void HtmlGuiComponent::mouseWheel(glm::detail::int32 xScroll, glm::detail::int32 yScroll)
{
	
}

void HtmlGuiComponent::textEvent(const wchar_t* evt, size_t evtLength)
{
	std::cout << "TEXT EVENT: " << evt << std::endl;

	if ( wcsncmp(evt, L"`", evtLength) == 0 || wcsncmp(evt, L"~", evtLength) == 0 )
	{
		/*
		 *	if( $('#console').hasClass('hidden') ) {
		 *		$('#console').removeClass('hidden');
		 *	} else {
		 *		$('#console').addClass('hidden');
		 *	}
		 */

		/*
		window_->executeJavascript(Berkelium::WideString::point_to(
									   L"if( $('#console').hasClass('hidden') ) {\
											$('#console').removeClass('hidden');\
											$('#console').addClass('visible');\
											$('#console').click();\
									   } else {\
											$('#console').addClass('hidden');\
											$('#console').removeClass('visible');\
									   }"
									   ));
									   */
	}
	else
	{
		std::cout << "HERE 1 " << (char*)evt << std::endl;
		//window_->focus();
		wchar_t outchars[2];
		outchars[0] = evt[0];
		outchars[1] = 0;
		std::cout << "HERE 2 " << outchars[0] << std::endl;
		//window_->textEvent(outchars, 1);
	}
}

void HtmlGuiComponent::keyEvent(bool pressed, glm::detail::int32 mods, glm::detail::int32 vk_code, glm::detail::int32 scancode)
{
	std::cout << "KEY EVENT: (" << pressed << ") " << (char)vk_code << std::endl;

	if ( vk_code == '`' || vk_code == '~' )
	{
		/*
		window_->executeJavascript(Berkelium::WideString::point_to(
									   L"if( $('#console').hasClass('hidden') ) {\
											$('#console').removeClass('hidden');\
											$('#console').addClass('visible');\
											$('#console').click();\
										} else {\
											$('#console').addClass('hidden');\
											$('#console').removeClass('visible');\
										}"
									   ));
									   */
	}
	else
	{
		//window_->focus();
		wchar_t outchars[2];
		outchars[0] = vk_code;
		outchars[1] = 0;
		std::cout << "HERE 2 " << outchars[0] << std::endl;
		//window_->textEvent(outchars, 1);
		//window_->keyEvent(pressed, mods, vk_code, scancode);
	}
}

/**
 * Need to implement!
 */
int HtmlGuiComponent::setContents(std::string contents)
{
	url_ = contents;

	return 0;
}

int HtmlGuiComponent::loadContentsFromFile(std::string filename)
{
	url_ = filename;

	return 0;
}

void HtmlGuiComponent::update()
{
	//window_->executeJavascript(Berkelium::WideString::point_to(L"update();"));
}


void HtmlGuiComponent::render(shaders::IShaderProgram* shader)
{
	//texture_->bind();
	//shader->bindVariableByBindingName( shaders::IShader::BIND_TYPE_TEXTURE, texture_->getBindPoint() );
	CefDoMessageLoopWork();
	
	if ( webTextureReady_ )
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glColor4f(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, web_texture);

		// display
		glBegin(GL_QUADS);
		glTexCoord2f(0.f, 1.f); glVertex3f(-1.f, -1.f, 0.f);
		glTexCoord2f(0.f, 0.f); glVertex3f(-1.f,  1.f, 0.f);
		glTexCoord2f(1.f, 0.f); glVertex3f(1.f,  1.f, 0.f);
		glTexCoord2f(1.f, 1.f); glVertex3f(1.f, -1.f, 0.f);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	// wait a bit before calling Berkelium::update() again
	//if (testint > 3) {
	//Berkelium::update();
	testint = -1;
	//}

	testint++;
}

/*
void HtmlGuiComponent::onCrashed(Berkelium::Window*win)
{
	std::cout << "*** onCrashed " << std::endl;
}

void HtmlGuiComponent::onUnresponsive(Berkelium::Window*win)
{
	std::cout << "*** onUnresponsive " << std::endl;
}

void HtmlGuiComponent::onScriptAlert(Berkelium::Window*win, Berkelium::WideString message, Berkelium::WideString defaultValue, Berkelium::URLString url, int flags, bool&success, Berkelium::WideString&value)
{
	std::wcout << L"*** onScriptAlert " << message << std::endl;
}
*/

std::wstring HtmlGuiComponent::getObjectName(std::wstring name)
{
	//return L"testObj";
	std::string::size_type loc = name.find(L".", 0);

	if ( loc != std::string::npos )
	{
		return name.substr(0, (int)loc);
	}

	return L"";
}

std::wstring HtmlGuiComponent::getFunctionName(std::wstring name)
{
	//return L"testFunc";
	std::string::size_type loc = name.find(L".", 0);

	if ( loc != std::string::npos )
	{
		return name.substr((int)loc + 1);
	}

	return L"";
}

/*
void HtmlGuiComponent::onJavascriptCallback(Berkelium::Window*win, void* replyMsg, Berkelium::URLString url, Berkelium::WideString funcName, Berkelium::Script::Variant*args, size_t numArgs)
{
	   std::cout << "*** onJavascriptCallback at URL " << url << ", "
	                  << (replyMsg?"synchronous":"async") << std::endl;
	   std::wcout << L"    Function name: " << funcName << std::endl;

	   for (size_t i = 0; i < numArgs; i++) {
	        Berkelium::WideString jsonStr = toJSON(args[i]);
	        std::wcout << L"    Argument " << i << ": ";
	        if (args[i].type() == Berkelium::Script::Variant::JSSTRING) {
	                std::wcout << L"(string) " << args[i].toString() << std::endl;
	        } else {
	                std::wcout << jsonStr << std::endl;
	        }
	        Berkelium::Script::toJSON_free(jsonStr);
	   }

	// parse object and function names
	const std::wstring berkeliumFuncName = std::wstring(funcName.data(), funcName.length());

	const std::wstring objName = getObjectName(berkeliumFuncName);
	const std::wstring functionName = getFunctionName(berkeliumFuncName);

	//std::wcout << objName << " " << functionName << std::endl;


	std::vector< CallbackParameter > params;

	// parse parameters
	for ( size_t i = 0; i < numArgs; i++ )
	{
		switch ( args[i].type())
		{
		case Berkelium::Script::Variant::JSSTRING: {
			std::wstring p = std::wstring(args[i].toString().data(), args[i].toString().length());
			params.push_back(CallbackParameter(p));
		}
		break;

		case Berkelium::Script::Variant::JSDOUBLE: {
			double p = args[i].toDouble();
			params.push_back(CallbackParameter(p));
		}
		break;

		case Berkelium::Script::Variant::JSBOOLEAN: {
			bool p = args[i].toBoolean();
			params.push_back(CallbackParameter(p));
		}
		break;

		case Berkelium::Script::Variant::JSNULL:

			break;

		default:
			// int i = args[i].toInteger();
			break;
		}
	}


	if ( guiObjects_[objName] != nullptr )
	{
		Berkelium::Script::Variant r = guiObjects_[objName]->processCallback(functionName, params);

		if ( replyMsg )
			win->synchronousScriptReturn(replyMsg, r);
	}
}
*/

/*
bool HtmlGuiComponent::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
	rect = CefRect(0, 0, 800, 600);
	return true;
}

void HtmlGuiComponent::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList &dirtyRects, const void *buffer, int width, int height)
{

	memcpy(texBuf->getCurrentLock().data, buffer, width*height*4);

}
*/

/*
void HtmlGuiComponent::onPaint(
		Berkelium::Window* wini,
		const unsigned char*bitmap_in, 
		const Berkelium::Rect&bitmap_rect,
		size_t num_copy_rects, 
		const Berkelium::Rect* copy_rects,
		int dx, 
		int dy, 
		const Berkelium::Rect& scroll_rect
	)
{
	bool updated = mapOnPaintToTexture(
		wini, 
		bitmap_in, 
		bitmap_rect, 
		num_copy_rects, 
		copy_rects,
		dx,
		dy, 
		scroll_rect,
		web_texture, 
		width_, 
		height_, 
		needs_full_refresh, 
		scroll_buffer
	);


	if ( updated )
	{
		needs_full_refresh = false;
		webTextureReady_ = true;
	}
}
*/

/** Handles an onPaint call by mapping the results into an OpenGL texture. The
 *  first parameters are the same as Berkelium::WindowDelegate::onPaint.  The
 *  additional parameters and return value are:
 *  \param dest_texture - the OpenGL texture handle for the texture to render
 *                        the results into.
 *  \param dest_texture_width - width of destination texture
 *  \param dest_texture_height - height of destination texture
 *  \param ignore_partial - if true, ignore any partial updates.  This is useful
 *         if you have loaded a new page, but updates for the old page have not
 *         completed yet.
 *  \param scroll_buffer - a temporary workspace used for scroll data.  Must be
 *         at least dest_texture_width * dest_texture_height * 4 bytes large.
 *  \returns true if the texture was updated, false otherwiase
 */
/*
bool HtmlGuiComponent::mapOnPaintToTexture(
		Berkelium::Window*wini,
		const unsigned char* bitmap_in, 
		const Berkelium::Rect& bitmap_rect,
		size_t num_copy_rects, 
		const Berkelium::Rect*copy_rects,
		int dx, 
		int dy,
		const Berkelium::Rect& scroll_rect,
		unsigned int dest_texture,
		unsigned int dest_texture_width,
		unsigned int dest_texture_height,
		bool ignore_partial,
		char* scroll_buffer
	)
{
	//BOOST_LOG_TRIVIAL(debug) << "mapOnPaintToTexture: " << dest_texture_width << "x" << dest_texture_width << " dest_texture: " << dest_texture;

	glBindTexture(GL_TEXTURE_2D, dest_texture);

	const int kBytesPerPixel = 4;

	// If we've reloaded the page and need a full update, ignore updates
	// until a full one comes in.  This handles out of date updates due to
	// delays in event processing.
	if ( ignore_partial )
	{
		if ( bitmap_rect.left() != 0 ||
			 bitmap_rect.top() != 0 ||
			 bitmap_rect.right() != dest_texture_width ||
			 bitmap_rect.bottom() != dest_texture_height )
		{
			return false;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, kBytesPerPixel, dest_texture_width, dest_texture_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, bitmap_in);
		ignore_partial = false;
		return true;
	}


	// Now, we first handle scrolling. We need to do this first since it
	// requires shifting existing data, some of which will be overwritten by
	// the regular dirty rect update.
	if ( dx != 0 || dy != 0 )
	{
		// scroll_rect contains the Rect we need to move
		// First we figure out where the the data is moved to by translating it
		Berkelium::Rect scrolled_rect = scroll_rect.translate(-dx, -dy);
		// Next we figure out where they intersect, giving the scrolled
		// region
		Berkelium::Rect scrolled_shared_rect = scroll_rect.intersect(scrolled_rect);
		// Only do scrolling if they have non-zero intersection
		if ( scrolled_shared_rect.width() > 0 && scrolled_shared_rect.height() > 0 )
		{
			// And the scroll is performed by moving shared_rect by (dx,dy)
			Berkelium::Rect shared_rect = scrolled_shared_rect.translate(dx, dy);

			int wid = scrolled_shared_rect.width();
			int hig = scrolled_shared_rect.height();

			if ( DEBUG_PAINT )
			{
				std::cout << "Scroll rect: w=" << wid << ", h=" << hig << ", ("
						  << scrolled_shared_rect.left() << "," << scrolled_shared_rect.top()
						  << ") by (" << dx << "," << dy << ")" << std::endl;
			}

			int inc = 1;
			char*outputBuffer = scroll_buffer;
			// source data is offset by 1 line to prevent memcpy aliasing
			// In this case, it can happen if dy==0 and dx!=0.
			char*inputBuffer = scroll_buffer + (dest_texture_width * 1 * kBytesPerPixel);
			int jj = 0;
			if ( dy > 0 )
			{
				// Here, we need to shift the buffer around so that we start in the
				// extra row at the end, and then copy in reverse so that we
				// don't clobber source data before copying it.
				outputBuffer = scroll_buffer + (
					(scrolled_shared_rect.top() + hig + 1) * dest_texture_width
					- hig * wid) * kBytesPerPixel;
				inputBuffer = scroll_buffer;
				inc = -1;
				jj = hig - 1;
			}

			// Copy the data out of the texture
			glGetTexImage(
				GL_TEXTURE_2D, 0,
				GL_BGRA, GL_UNSIGNED_BYTE,
				inputBuffer
				);

			// Annoyingly, OpenGL doesn't provide convenient primitives, so
			// we manually copy out the region to the beginning of the
			// buffer
			for (; jj < hig && jj >= 0; jj += inc )
			{
				memcpy(
					outputBuffer + (jj * wid) * kBytesPerPixel,
				    //scroll_buffer + (jj*wid * kBytesPerPixel),
					inputBuffer + (
						(scrolled_shared_rect.top() + jj) * dest_texture_width
						+ scrolled_shared_rect.left()) * kBytesPerPixel,
					wid * kBytesPerPixel
					);
			}

			// And finally, we push it back into the texture in the right
			// location
			glTexSubImage2D(GL_TEXTURE_2D, 0,
							shared_rect.left(), shared_rect.top(),
							shared_rect.width(), shared_rect.height(),
							GL_BGRA, GL_UNSIGNED_BYTE, outputBuffer
							);
		}
	}

	//BOOST_LOG_TRIVIAL(debug) << "mapOnPaintToTexture: here 2";
	if ( DEBUG_PAINT )
	{
		std::cout << (void*)wini << " Bitmap rect: w="
				  << bitmap_rect.width() << ", h=" << bitmap_rect.height()
				  << ", (" << bitmap_rect.top() << "," << bitmap_rect.left()
				  << ") tex size " << dest_texture_width << "x" << dest_texture_height
				  << std::endl;
	}

	//BOOST_LOG_TRIVIAL(debug) << "mapOnPaintToTexture: here 3";
	for ( size_t i = 0; i < num_copy_rects; i++ )
	{
		int wid = copy_rects[i].width();
		int hig = copy_rects[i].height();
		int top = copy_rects[i].top() - bitmap_rect.top();
		int left = copy_rects[i].left() - bitmap_rect.left();
		//BOOST_LOG_TRIVIAL(debug) << "mapOnPaintToTexture: here 4";

		if ( DEBUG_PAINT )
		{
			std::cout << (void*)wini << " Copy rect: w=" << wid << ", h=" << hig << ", ("
					  << top << "," << left << ")" << std::endl;
		}
		//BOOST_LOG_TRIVIAL(debug) << "mapOnPaintToTexture: here 5";
		for ( int jj = 0; jj < hig; jj++ )
		{
			memcpy(
				scroll_buffer + jj * wid * kBytesPerPixel,
				bitmap_in + (left + (jj + top) * bitmap_rect.width()) * kBytesPerPixel,
				wid * kBytesPerPixel
				);
		}
		//BOOST_LOG_TRIVIAL(debug) << "mapOnPaintToTexture: here 6";
		// Finally, we perform the main update, just copying the rect that is
		// marked as dirty but not from scrolled data.
		glTexSubImage2D(GL_TEXTURE_2D, 0,
						copy_rects[i].left(), copy_rects[i].top(),
						wid, hig,
						GL_BGRA, GL_UNSIGNED_BYTE, scroll_buffer
						);
		//BOOST_LOG_TRIVIAL(debug) << "mapOnPaintToTexture: here 7";
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	//BOOST_LOG_TRIVIAL(debug) << "mapOnPaintToTexture: here 8";

	return true;
}
*/

void HtmlGuiComponent::executeScript(std::wstring script)
{
	//window_->executeJavascript(Berkelium::WideString::point_to(script));
}

bool HtmlGuiComponent::isVisible()
{
	return isVisible_;
}

void HtmlGuiComponent::setVisible(bool isVisible)
{
	isVisible_ = isVisible;
}

IGUIObject* HtmlGuiComponent::createGUIObject(std::wstring name)
{
	if ( guiObjects_[name] != nullptr )
	{
		return nullptr;
	}
	
	// TODO: Re-implement using CEF3
	guiObjects_[name] = std::unique_ptr<GUIObject>(new GUIObject(name));

	return guiObjects_[name].get();
	
	return nullptr;
}

IGUIObject* HtmlGuiComponent::getGUIObject(std::wstring name)
{
	if ( guiObjects_[name] == nullptr )
	{
		return nullptr;
	}

	return guiObjects_[name].get();
}
}
}

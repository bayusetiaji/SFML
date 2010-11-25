/* rbSFML - Copyright (c) 2010 Henrik Valter Vogelius Hansson - groogy@groogy.se
 * This software is provided 'as-is', without any express or
 * implied warranty. In no event will the authors be held
 * liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute
 * it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented;
 *    you must not claim that you wrote the original software.
 *    If you use this software in a product, an acknowledgment
 *    in the product documentation would be appreciated but
 *    is not required.
 *
 * 2. Altered source versions must be plainly marked as such,
 *    and must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any
 *    source distribution.
 */
 
#include "SoundStream.hpp"
#include "main.hpp"
#include <SFML/Audio/SoundStream.hpp>

VALUE globalSoundRecorderClass;

class rbSoundRecorder : public sf::SoundRecorder
{
public:
	SoundRecorder()
	{
	}
	
	void Init( VALUE rubySelf )
	{
		mySelf = rubySelf;
		myOnStartID = rb_intern( "onStart" );
		myOnStopID = rb_intern( "onStop" );
		myOnProcessSamplesID = rb_intern( "onProcessSamples" );
	}
	
protected:
	virtual bool OnStart()
	{
		if( rb_respond_to( myOnStartID ) == 0 )
		{
			return true;
		}
		else
		{
			if( rb_funcall( mySelf, myOnStartID, 0 ) == Qfalse )
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}
	
	virtual void OnStop()
	{
		if( rb_respond_to( myOnStopID ) != 0 )
		{
			rb_funcall( mySelf, myOnStopID, 0 );
		}
	}
	
	virtual bool OnProcessSamples( const sf::Int16 *someSamples, std::size_t someCount )
	{
		if( rb_respond_to( myOnProcessSamples ) == 0 )
		{
			return true;
		}
		else
		{
			if( rb_funcall( mySelf, myOnProcessSamples, 0 ) == Qfalse )
			{
				return false;
			}
			else
			{
				return true;
			}
		}
	}
	
	VALUE mySelf;
	ID myOnStartID;
	ID myOnStopID;
	ID myOnProcessSamplesID;
};


static void SoundRecorder_Free( rbSoundRecorder * anObject )
{
	delete anObject;
}

static VALUE SoundRecorder_Start( int argc, VALUE *args, VALUE self )
{
	sf::SoundRecorder *object = NULL;
	Data_Get_Struct( self, sf::SoundRecorder, object );
	unsigned int sampleRate = 44100;
	switch( argc )
	{
		case 1:
			sampleRate = FIX2UINT( args[0] );
		case 0:
			object->Start( sampleRate );
			break;
		default:
			rb_raise( rb_eArgError, "Expected 0 or 1 arguments but was given %d", argc );
	}
	return Qnil;
}

static VALUE SoundRecorder_Stop( VALUE self )
{
	sf::SoundRecorder *object = NULL;
	Data_Get_Struct( self, sf::SoundRecorder, object );
	object->Stop();
	return Qnil;
}

static VALUE SoundRecorder_GetSampleRate( VALUE self )
{
	sf::SoundRecorder *object = NULL;
	Data_Get_Struct( self, sf::SoundRecorder, object );
	return INT2FIX( object->GetSampleRate() );
}

static VALUE SoundRecorder_New( int argc, VALUE *args, VALUE aKlass )
{
	rbSoundRecorder *object = new rbSoundRecorder();
	VALUE rbData = Data_Wrap_Struct( aKlass, 0, SoundRecorder_Free, object );
	rb_obj_call_init( rbData, argc, args );
	return rbData;
}

static VALUE SoundRecorder_IsAvailable( VALUE aKlass )
{
	return ( sf::SoundRecorder::IsAvailable() == true ? Qtrue : Qfalse );
}

void Init_SoundRecorder( void )
{
/* SFML namespace which contains the classes of this module. */
	VALUE sfml = rb_define_module( "SFML" );
/* Abstract base class for capturing sound data.
 *
 * SFML::SoundRecorder provides a simple interface to access the audio recording capabilities of the computer 
 * (the microphone).
 *
 * As an abstract base class, it only cares about capturing sound samples, the task of making something useful with 
 * them is left to the derived class. Note that SFML provides a built-in specialization for saving the captured data 
 * to a sound buffer (see sf::SoundBufferRecorder).
 *
 * A derived class has only one virtual function to override:
 *
 *   - onProcessSamples provides the new chunks of audio samples while the capture happens
 *
 * Moreover, two additionnal virtual functions can be overriden as well if necessary:
 *
 *   - onStart is called before the capture happens, to perform custom initializations
 *   - onStop is called after the capture ends, to perform custom cleanup
 *
 * The audio capture feature may not be supported or activated on every platform, thus it is recommended to check 
 * its availability with the isAvailable() function. If it returns false, then any attempt to use an audio recorder 
 * will fail.
 *
 * It is important to note that the audio capture happens in a separate thread, so that it doesn't block the rest of 
 * the program. In particular, the OnProcessSamples and OnStop virtual functions (but not OnStart) will be called from 
 * this separate thread. It is important to keep this in mind, because you may have to take care of synchronization 
 * issues if you share data between threads.
 *
 * Usage example:
 *
 *   class CustomRecorder < SFML::SoundRecorder
 *     def onStart() # optional
 *       # Initialize whatever has to be done before the capture starts
 *       ...
 *
 *       # Return true to start playing
 *       return true
 *     end
 *
 *     def onProcessSamples( samples, samplesCount )
 *       # Do something with the new chunk of samples (store them, send them, ...)
 *       ...
 *
 *       # Return true to continue playing
 *       return true
 *     end
 * 
 *     def onStop() # optional
 *       # Clean up whatever has to be done after the capture ends
 *       ...
 *     end
 *   end
 * 
 *   # Usage
 *   if CustomRecorder.isAvailable()
 *     recorder = CustomRecorder.new
 *     recorder.start()
 *     ...
 *     recorder.stop()
 *   end
 */
	globalSoundRecorderClass = rb_define_class_under( sfml, "SoundRecorder", rb_cObject );
	
	// Class methods
	rb_define_singleton_method( globalSoundRecorderClass, "new", SoundRecorder_New, -1 );
	rb_define_singleton_method( globalSoundRecorderClass, "isAvailable", SoundRecorder_IsAvailable, 0 );
	
	// Instance methods
	rb_define_method( globalSoundRecorderClass, "start", SoundRecorder_Start, -1 );
	rb_define_method( globalSoundRecorderClass, "stop", SoundRecorder_Stop, 0 );
	rb_define_method( globalSoundRecorderClass, "getSampleRate", SoundRecorder_GetSampleRate, 0 );
		
	// Class Aliases
	rb_define_alias( globalSoundRecorderClass, "is_available", "isAvailable" );
	rb_define_alias( globalSoundRecorderClass, "available?", "isAvailable" );
	
	// Instance Aliases
	rb_define_alias( globalSoundRecorderClass, "get_sample_rate", "getSampleRate" );
	rb_define_alias( globalSoundRecorderClass, "sampleRate", "getSampleRate" );
	rb_define_alias( globalSoundRecorderClass, "sample_rate", "getSampleRate" );
}

/*
 * mixer.h - audio-device-independent mixer for LMMS
 *
 * Copyright (c) 2004-2006 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * 
 * This file is part of Linux MultiMedia Studio - http://lmms.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */


#ifndef _MIXER_H
#define _MIXER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "qt3support.h"

#ifdef QT4

#include <QMutex>
#include <QVector>

#else

#include <qobject.h>
#include <qmutex.h>
#include <qvaluevector.h>

#endif


#include "types.h"
#include "volume.h"
#include "panning.h"
#include "note.h"
#include "play_handle.h"
#include "effect_board.h"


class audioDevice;
class midiClient;
class lmmsMainWin;
class plugin;
class audioPort;


const int DEFAULT_BUFFER_SIZE = 512;

const Uint8  DEFAULT_CHANNELS = 2;

const Uint8  SURROUND_CHANNELS =
#ifndef DISABLE_SURROUND
				4;
#else
				2;
#endif

const Uint8  QUALITY_LEVELS = 2;
const Uint32 DEFAULT_QUALITY_LEVEL = 0;
const Uint32 HIGH_QUALITY_LEVEL = DEFAULT_QUALITY_LEVEL+1;
extern Uint32 SAMPLE_RATES[QUALITY_LEVELS];
const Uint32 DEFAULT_SAMPLE_RATE = 44100;


typedef sampleType sampleFrame[DEFAULT_CHANNELS];
typedef sampleType surroundSampleFrame[SURROUND_CHANNELS];

typedef struct
{
	float vol[SURROUND_CHANNELS];
} volumeVector;


const Uint32 BYTES_PER_SAMPLE = sizeof( sampleType );
const Uint32 BYTES_PER_FRAME = sizeof( sampleFrame );
const Uint32 BYTES_PER_SURROUND_FRAME = sizeof( surroundSampleFrame );
const Uint32 BYTES_PER_OUTPUT_SAMPLE = sizeof( outputSampleType );

const float OUTPUT_SAMPLE_MULTIPLIER = 32767.0f;


const float BASE_FREQ = 440.0f;
const tones BASE_TONE = A;
const octaves BASE_OCTAVE = OCTAVE_4;



class mixer : public QObject
{
	Q_OBJECT
public:
	static inline mixer * inst( void )
	{
		if( s_instanceOfMe == NULL )
		{
			s_instanceOfMe = new mixer();
		}
		return( s_instanceOfMe );
	}


	void FASTCALL bufferToPort( sampleFrame * _buf, Uint32 _frames,
						Uint32 _framesAhead,
						volumeVector & _volumeVector,
						audioPort * _port );
	inline Uint32 framesPerAudioBuffer( void ) const
	{
		return( m_framesPerAudioBuffer );
	}

	inline Uint8 cpuLoad( void ) const
	{
		return( m_cpuLoad );
	}

	inline bool highQuality( void ) const
	{
		return( m_qualityLevel > DEFAULT_QUALITY_LEVEL );
	}


	inline const surroundSampleFrame * currentAudioBuffer( void ) const
	{
		return( m_curBuf );
	}

	// audio-device-stuff
	inline const QString & audioDevName( void ) const
	{
		return( m_audioDevName );
	}

	void FASTCALL setAudioDevice( audioDevice * _dev, bool _hq );
	void restoreAudioDevice( void );
	inline audioDevice * audioDev( void )
	{
		return( m_audioDev );
	}


	// audio-port-stuff
	inline void addAudioPort( audioPort * _port )
	{
		pause();
		m_audioPorts.push_back( _port );
		play();
	}

	inline void removeAudioPort( audioPort * _port )
	{
		vvector<audioPort *>::iterator it = qFind( m_audioPorts.begin(),
							m_audioPorts.end(),
							_port );
		if( it != m_audioPorts.end() )
		{
			m_audioPorts.erase( it );
		}
	}

	// MIDI-client-stuff
	inline const QString & midiClientName( void ) const
	{
		return( m_midiClientName );
	}

	inline midiClient * getMIDIClient( void )
	{
		return( m_midiClient );
	}


	inline void addPlayHandle( playHandle * _ph )
	{
		m_playHandles.push_back( _ph );
	}

	inline void removePlayHandle( playHandle * _ph )
	{
		m_playHandlesToRemove.push_back( _ph );
	}

	inline const playHandleVector & playHandles( void ) const
	{
		return( m_playHandles );
	}

	void checkValidityOfPlayHandles( void );



	inline Uint32 sampleRate( void )
	{
		return( SAMPLE_RATES[m_qualityLevel] );
	}


	inline float masterGain( void ) const
	{
		return( m_masterGain );
	}

	inline void setMasterGain( float _mo )
	{
		m_masterGain = _mo;
	}


	static inline sampleType clip( sampleType _s )
	{
		if( _s > 1.0f )
		{
			return( 1.0f );
		}
		else if( _s < -1.0f )
		{
			return( -1.0f );
		}
		return( _s );
	}


	void pause( void )
	{
		m_mixMutex.lock();
	}

	void play( void )
	{
		m_mixMutex.unlock();
	}


	void FASTCALL clear( bool _everything = FALSE );


	void FASTCALL clearAudioBuffer( sampleFrame * _ab, Uint32 _frames );
#ifndef DISABLE_SURROUND
	void FASTCALL clearAudioBuffer( surroundSampleFrame * _ab,
							Uint32 _frames );
#endif

	inline bool haveNoRunningNotes( void ) const
	{
		return( m_playHandles.size() == 0 );
	}


	const surroundSampleFrame * renderNextBuffer( void );

public slots:
	void setHighQuality( bool _hq_on = FALSE );


signals:
	void sampleRateChanged( void );
	void nextAudioBuffer( const surroundSampleFrame *, Uint32 _frames );


private:

	static mixer * s_instanceOfMe;

	mixer();
	~mixer();

	void stopProcessing( void );


	// we don't allow to create mixer by using copy-ctor
	mixer( const mixer & )
	{
	}



	audioDevice * tryAudioDevices( void );
	midiClient * tryMIDIClients( void );

	void processBuffer( surroundSampleFrame * _buf, fxChnl _fx_chnl );



	vvector<audioPort *> m_audioPorts;

	Uint32 m_framesPerAudioBuffer;

	surroundSampleFrame * m_curBuf;
	surroundSampleFrame * m_nextBuf;

	Uint8 m_cpuLoad;

	playHandleVector m_playHandles;
	playHandleVector m_playHandlesToRemove;

	Uint8 m_qualityLevel;
	float m_masterGain;


	audioDevice * m_audioDev;
	audioDevice * m_oldAudioDev;
	QString m_audioDevName;


	midiClient * m_midiClient;
	QString m_midiClientName;


	QMutex m_mixMutex;


	friend class lmmsMainWin;

} ;


#endif

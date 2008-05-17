/*
 * patman.h - header for a GUS-compatible patch instrument plugin
 *
 * Copyright (c) 2007-2008 Javier Serrano Polo <jasp00/at/users.sourceforge.net>
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */


#ifndef _PATMAN_H_
#define _PATMAN_H_


#include "instrument.h"
#include "instrument_view.h"
#include "sample_buffer.h"
#include "automatable_model.h"


class pixmapButton;


#define MODES_16BIT	( 1 << 0 )
#define MODES_UNSIGNED	( 1 << 1 )
#define MODES_LOOPING	( 1 << 2 )
#define MODES_PINGPONG	( 1 << 3 )
#define MODES_REVERSE	( 1 << 4 )
#define MODES_SUSTAIN	( 1 << 5 )
#define MODES_ENVELOPE	( 1 << 6 )
#define MODES_CLAMPED	( 1 << 7 )


class patmanInstrument : public instrument
{
	Q_OBJECT
public:
	class subPluginFeatures : public plugin::descriptor::subPluginFeatures
	{
	public:
		subPluginFeatures( plugin::PluginTypes _type );

		virtual const QStringList & supportedExtensions( void )
		{
			return( supported_extensions() );
		}

		static const QStringList & supported_extensions( void );

	} ;


	patmanInstrument( instrumentTrack * _track );
	virtual ~patmanInstrument();

	virtual void playNote( notePlayHandle * _n, bool _try_parallelizing,
						sampleFrame * _working_buffer );
	virtual void deleteNotePluginData( notePlayHandle * _n );


	virtual void saveSettings( QDomDocument & _doc, QDomElement & _parent );
	virtual void loadSettings( const QDomElement & _this );

	virtual void setParameter( const QString & _param,
						const QString & _value );

	virtual QString nodeName( void ) const;

	virtual pluginView * instantiateView( QWidget * _parent );


public slots:
	void setFile( const QString & _patch_file, bool _rename = TRUE );


private:
	typedef struct
	{
		sampleBuffer::handleState * state;
		bool tuned;
		sampleBuffer * sample;
	} handle_data;

	QString m_patchFile;
	QVector<sampleBuffer *> m_patchSamples;
	boolModel m_loopedModel;
	boolModel m_tunedModel;


	enum LoadErrors
	{
		LoadOK,
		LoadOpen,
		LoadNotGUS,
		LoadInstruments,
		LoadLayers,
		LoadIO
	} ;

	LoadErrors loadPatch( const QString & _filename );
	void unloadCurrentPatch( void );

	void selectSample( notePlayHandle * _n );


	friend class patmanView;

signals:
	void fileChanged( void );

} ;



class patmanView : public instrumentView
{
	Q_OBJECT
public:
	patmanView( instrument * _instrument, QWidget * _parent );
	virtual ~patmanView();


public slots:
	void openFile( void );
	void updateFilename( void );


protected:
	virtual void dragEnterEvent( QDragEnterEvent * _dee );
	virtual void dropEvent( QDropEvent * _de );
	virtual void paintEvent( QPaintEvent * );


private:
	virtual void modelChanged( void );

	patmanInstrument * m_pi;
	QString m_displayFilename;

	pixmapButton * m_openFileButton;
	pixmapButton * m_loopButton;
	pixmapButton * m_tuneButton;

} ;



#endif

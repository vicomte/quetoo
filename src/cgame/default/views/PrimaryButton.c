/*
 * Copyright(c) 1997-2001 id Software, Inc.
 * Copyright(c) 2002 The Quakeforge Project.
 * Copyright(c) 2006 Quetoo.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "cg_local.h"

#include "PrimaryButton.h"

#define _Class _PrimaryButton

#pragma mark - View

/**
 * @see View::respondToEvent(View *, const SDL_Event *)
 */
static void respondToEvent(View *self, const SDL_Event *event) {

	super(View, self, respondToEvent, event);

	((Control *) self)->bevel = ControlBevelTypeNone;
}

#pragma mark - PrimaryButton

/**
 * @fn PrimaryButton *PrimaryButton::init(PrimaryButton *self)
 *
 * @memberof PrimaryButton
 */
static PrimaryButton *initWithFrame(PrimaryButton *self, const SDL_Rect *frame, ControlStyle style) {

	self = (PrimaryButton *) super(Button, self, initWithFrame, frame, style);
	if (self) {
		$(self->button.title, setFont, $$(Font, defaultFont, FontCategoryPrimaryResponder));

		self->button.control.bevel = ControlBevelTypeNone;

		self->button.control.view.borderWidth = 1;
	}

	return self;
}

#pragma mark - Class lifecycle

/**
 * @see Class::initialize(Class *)
 */
static void initialize(Class *clazz) {

	((ViewInterface *) clazz->def->interface)->respondToEvent = respondToEvent;

	((PrimaryButtonInterface *) clazz->def->interface)->initWithFrame = initWithFrame;
}

/**
 * @fn Class *PrimaryButton::_PrimaryButton(void)
 * @memberof PrimaryButton
 */
Class *_PrimaryButton(void) {
	static Class clazz;
	static Once once;

	do_once(&once, {
		clazz.name = "PrimaryButton";
		clazz.superclass = _Button();
		clazz.instanceSize = sizeof(PrimaryButton);
		clazz.interfaceOffset = offsetof(PrimaryButton, interface);
		clazz.interfaceSize = sizeof(PrimaryButtonInterface);
		clazz.initialize = initialize;
	});

	return &clazz;
}

#undef _Class


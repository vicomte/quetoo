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

#include "cl_local.h"

/**
 * @brief Prepares the collision model to clip to the specified entity. For
 * mesh models, the box hull must be set to reflect the bounds of the entity.
 */
static int32_t Cl_HullForEntity(const entity_state_t *s) {

	if (s->solid == SOLID_BSP) {
		const cm_bsp_model_t *mod = cl.cm_models[s->model1];

		if (!mod) {
			Com_Error(ERROR_DROP, "SOLID_BSP with no model\n");
		}

		return mod->head_node;
	}

	const cl_entity_t *ent = &cl.entities[s->number];

	if (s->client) {
		return Cm_SetBoxHull(ent->mins, ent->maxs, CONTENTS_MONSTER);
	} else {
		return Cm_SetBoxHull(ent->mins, ent->maxs, CONTENTS_SOLID);
	}
}

/**
 * @brief Yields the contents mask (bitwise OR) for the specified point. The
 * world model and all solids are checked.
 */
int32_t Cl_PointContents(const vec3_t point) {

	int32_t contents = Cm_PointContents(point, 0);

	for (uint16_t i = 0; i < cl.frame.num_entities; i++) {

		const uint32_t snum = (cl.frame.entity_state + i) & ENTITY_STATE_MASK;
		const entity_state_t *s = &cl.entity_states[snum];

		if (s->solid < SOLID_BOX) {
			continue;
		}

		const cl_entity_t *ent = &cl.entities[s->number];

		if (ent == cl.entity) {
			continue;
		}

		const int32_t head_node = Cl_HullForEntity(s);

		contents |= Cm_TransformedPointContents(point, head_node, &ent->inverse_matrix);
	}

	return contents;
}

/**
 * @brief A structure facilitating clipping to SOLID_BOX entities.
 */
typedef struct {
	const vec_t *mins, *maxs;
	const vec_t *start, *end;
	vec3_t box_mins, box_maxs;
	cm_trace_t trace;
	uint16_t skip;
	int32_t contents;
} cl_trace_t;

/**
 * @brief Clips the specified trace to other solid entities in the frame.
 */
static void Cl_ClipTraceToEntities(cl_trace_t *trace) {

	for (uint16_t i = 0; i < cl.frame.num_entities; i++) {

		const uint32_t snum = (cl.frame.entity_state + i) & ENTITY_STATE_MASK;
		const entity_state_t *s = &cl.entity_states[snum];

		if (s->solid < SOLID_BOX) {
			continue;
		}

		if (s->number == trace->skip) {
			continue;
		}

		const cl_entity_t *ent = &cl.entities[s->number];

		if (ent == cl.entity) {
			continue;
		}

		if (!BoxIntersect(ent->abs_mins, ent->abs_maxs, trace->box_mins, trace->box_maxs)) {
			continue;
		}

		const int32_t head_node = Cl_HullForEntity(s);

		cm_trace_t tr = Cm_TransformedBoxTrace(trace->start, trace->end, trace->mins, trace->maxs,
		                                       head_node, trace->contents, &ent->matrix, &ent->inverse_matrix);

		if (tr.start_solid || tr.fraction < trace->trace.fraction) {
			trace->trace = tr;
			trace->trace.ent = (struct g_entity_s *) (intptr_t) s->number;

			if (tr.start_solid) {
				return;
			}
		}
	}
}

/**
 * @brief Client-side collision model tracing. This is the reciprocal of
 * Sv_Trace.
 *
 * @param skip An optional entity number for which all tests are skipped. Pass
 * 0 for none, because entity 0 is the world, which we always test.
 */
cm_trace_t Cl_Trace(const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs,
                    const uint16_t skip, const int32_t contents) {

	cl_trace_t trace;

	memset(&trace, 0, sizeof(trace));

	if (!mins) {
		mins = vec3_origin;
	}
	if (!maxs) {
		maxs = vec3_origin;
	}

	// clip to world
	trace.trace = Cm_BoxTrace(start, end, mins, maxs, 0, contents);
	if (trace.trace.fraction < 1.0) {
		trace.trace.ent = (struct g_entity_s *) (intptr_t) - 1;

		if (trace.trace.start_solid) { // blocked entirely
			return trace.trace;
		}
	}

	trace.start = start;
	trace.end = end;
	trace.mins = mins;
	trace.maxs = maxs;
	trace.skip = skip;
	trace.contents = contents;

	Cm_TraceBounds(start, end, mins, maxs, trace.box_mins, trace.box_maxs);

	Cl_ClipTraceToEntities(&trace);

	return trace.trace;
}

/**
 * @brief Entry point for client-side prediction. For each server frame, run
 * the player movement code with the user commands we've sent to the server
 * but have not yet received acknowledgment for. Store the resulting move so
 * that it may be interpolated into by Cl_UpdateView.
 *
 * Most of the work is passed off to the client game, which is responsible for
 * the implementation Pm_Move.
 */
void Cl_PredictMovement(void) {

	if (!cls.cgame->UsePrediction()) {
		return;
	}

	const uint32_t last = cls.net_chan.outgoing_sequence;
	uint32_t ack = cls.net_chan.incoming_acknowledged;

	// if we are too far out of date, just freeze in place
	if (last - ack >= CMD_BACKUP) {
		Com_Debug(DEBUG_CLIENT, "Exceeded CMD_BACKUP\n");
		return;
	}

	GList *cmds = NULL;

	while (++ack <= last) {
		cmds = g_list_append(cmds, &cl.cmds[ack & CMD_MASK]);
	}

	cls.cgame->PredictMovement(cmds);

	g_list_free(cmds);
}

/**
 * @brief Checks for client side prediction errors. These will occur under normal gameplay
 * conditions if the client is pushed by another entity on the server (projectile, platform, etc.).
 */
void Cl_CheckPredictionError(void) {

	if (!cls.cgame->UsePrediction()) {
		return;
	}

	cl_predicted_state_t *pr = &cl.predicted_state;

	if (cl.delta_frame) {

		// calculate the last cl_cmd_t we sent that the server has processed
		const uint32_t cmd = cls.net_chan.incoming_acknowledged & CMD_MASK;

		// subtract what the server returned with what we had predicted it to be
		VectorSubtract(cl.frame.ps.pm_state.origin, pr->origins[cmd], pr->error);

		// if the error is too large, it was likely a teleport or respawn, so ignore it
		const vec_t len = VectorLength(pr->error);
		if (len > 64.0) {
			Com_Debug(DEBUG_CLIENT, "Clear %s\n", vtos(pr->error));
			VectorClear(pr->error);
		} else if (len > 0.1) {
			Com_Debug(DEBUG_CLIENT, "Error %s\n", vtos(pr->error));
		}

	} else {
		Com_Debug(DEBUG_CLIENT, "No delta\n");

		VectorCopy(cl.frame.ps.pm_state.origin, pr->view.origin);

		UnpackVector(cl.frame.ps.pm_state.view_offset, pr->view.offset);
		UnpackAngles(cl.frame.ps.pm_state.view_angles, pr->view.angles);

		VectorClear(pr->error);
	}
}

/**
 * @brief Ensures client-side prediction has the current collision model at its
 * disposal.
 */
void Cl_UpdatePrediction(void) {

	// ensure the world model is loaded
	if (!Com_WasInit(QUETOO_SERVER) || cl.demo_server || !Cm_NumModels()) {
		int64_t bs;

		const char *bsp_name = cl.config_strings[CS_MODELS];
		const int64_t bsp_size = strtoll(cl.config_strings[CS_BSP_SIZE], NULL, 10);

		Cm_LoadBspModel(bsp_name, &bs);

		if (bs != bsp_size) {
			Com_Error(ERROR_DROP, "Local map version differs from server: "
			          "%" PRId64 " != %" PRId64 "\n", bs, bsp_size);
		}
	}

	// load the BSP models for prediction as well
	for (uint16_t i = 1; i < MAX_MODELS; i++) {

		const char *s = cl.config_strings[CS_MODELS + i];
		if (*s == '*') {
			cl.cm_models[i] = Cm_Model(cl.config_strings[CS_MODELS + i]);
		} else {
			cl.cm_models[i] = NULL;
		}
	}
}

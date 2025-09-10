#include "game.h"

/*
	TODO: Animations
*/

function void
hv_append(SpriteBatch *sb, Vec4 sprpos, Vec4 atlpos, Vec3 color)
{
    sb->sprite[sb->count].pos = 	{ sprpos.x, sprpos.y };
    sb->sprite[sb->count].size = 	{ sprpos.z, sprpos.w };
    sb->sprite[sb->count].aoffset = {atlpos.x, atlpos.y};
    sb->sprite[sb->count].asize = 	{atlpos.z, atlpos.w};
    sb->sprite[sb->count].color = 	color;
    sb->count++;
}

function void
draw_text(SpriteBatch *sb, Vec2 pos, String8 text, Vec3 color = vec3(1.0f))
{
	f32 startx = pos.x;
	f32 starty = pos.y;
    
	for (usize i = 0; i < text.len; ++i) {
		AtlasGlyph fnt = atlas_glyphs[0];
		u8 letter = text[i];
        
		//fnt = glyphs[letter - 32];
		for(int j = 0; j < (int)array_len(atlas_glyphs); j++) {
			if(atlas_glyphs[j].value == letter) {
				fnt = atlas_glyphs[j];
				break;
			}
		}
        
		if(letter == '\n') {
			starty += fnt.rect.y + (f32)fnt.offset_y;
			startx = pos.x;
			continue;
		}
        
		f32 startx1 = startx + (f32)fnt.offset_x;
		f32 starty1 = starty + (f32)fnt.offset_y;
        
		f32 endx = fnt.rect.z;
		f32 endy = fnt.rect.w;
        
		hv_append(sb, (Vec4){startx1, starty1, endx, endy}, fnt.rect, color);
        
		startx += (f32)fnt.advance_x;
	}
}

function inline void
draw_rectangle(SpriteBatch *sb, Vec4 pos, Vec3 color = vec3(1.0f)) {
    hv_append(sb, pos, atlas_glyphs[array_len(atlas_glyphs) - 8].rect, color);
}

function inline void
draw_sprite(SpriteBatch *sb, Vec4 possize, TextureName tname,Vec3 color = vec3(1.0f))
{
	hv_append(sb, possize, atlas_textures[tname].rect, color);
}

function inline void
draw_sprite_flip_x(SpriteBatch *sb, Vec4 possize, TextureName tname,  Vec3 color = vec3(1.0f))
{
    Vec4 trect = atlas_textures[tname].rect;
	trect.x = trect.x + trect.z;
	trect.z = -trect.z;
	//draw_sprite_flip_x(m->sb, {lstate->pos.x, lstate->pos.y, 32, 32}, trect);
	hv_append(sb, possize, trect, color);
}

function inline void
draw_sprite_v2(SpriteBatch *sb, Vec2 pos, TextureName tname,Vec3 color = vec3(1.0f))
{
    Vec4 sprite = atlas_textures[tname].rect;
	Vec4 sb_pos = {pos.x, pos.y, sprite.z, sprite.w};
	hv_append(sb, sb_pos, sprite, color);
}

//function inline void
//anim_helper(usize *animate_index, usize animation_index)
//{
//		if (*animate_index < animations[animation_index].first_frame
//		   || *animate_index > animations[animation_index].last_frame)
//		{
//			*animate_index = animations[animation_index].first_frame;
//		}
//}

function void
init_game(Memory *mem)
{
	set_console(mem->log);
	mem->is_init = true;
	mem->state = (GameState*)arena_alloc(mem->permanent, sizeof(GameState));
    
	mem->state->pos = vec2(75.0f);
	mem->state->pos.x = 64.0f;
    
	mem->state->vel = vec2(0.0f);
}

function void
handle_input(GameState *state, ControllerInput *kinput, f32 dt)
{
	if(kinput->move_right.ended_down) {
		state->vel.x += 240.f;
		state->flip = false;
        
	} else if(kinput->move_left.ended_down) {
		state->vel.x -= 240.f;
		state->flip = true;
	}
    
	if(kinput->move_down.ended_down) {
		state->vel.y = 120.f;
	} else if(kinput->move_up.ended_down) {
		state->vel.y = -120.f;
	}
    
	if(was_pressed(&kinput->action_up) && state->grounded) {
		state->vel.y = -600.f;
		state->grounded = false;
	}
}

// NOTE(vitalis): if working on linux do not attempt to add optional args while hot reloading
UPDATE_FUNC(game_update)
{
	if(m->is_init == 2) {
		set_console(m->log);
        log_info("Gamecode reloaded");
		m->is_init = true;
	}
    if(!m->is_init) {
		init_game(m);
    	log_info("Initializing Gamecode");
    	m->sb = (SpriteBatch *)arena_alloc(m->permanent, sizeof(SpriteBatch));
	}
    
	m->sb->sprite = (Sprite*)arena_alloc(m->transient, MAX_SPRITES * sizeof(Sprite));
	m->sb->count = 0;
    
	GameState *lstate = m->state;
	ControllerInput *kinput = &m->input->cinput[HV_Keyboard];
    
    Assert(m->state != nil);
    Assert(kinput != nil);
    Assert(m->transient != nil);
    
	lstate->vel.y += 1000 * m->dt;
	handle_input(lstate, kinput, m->dt);
    
	//TODO: Reset dugme
    if (kinput && was_pressed(&kinput->start)) {
        lstate->pos = {65.0f, 65.0f};
        lstate->vel = vec2(0.0f);
	}
    
	Vec2 nextpos = lstate->pos + (lstate->vel * m->dt);
    
	if(nextpos.y + 32.f >= 500 && nextpos.x + 32.f >= 50.f && nextpos.x < 250.f) {
        nextpos.y = 500.f - 32.f;
        lstate->vel.y = 0;
        lstate->grounded = true;
	}
    
	lstate->pos = nextpos;
	lstate->vel.x = 0.0f;
    
	draw_rectangle(m->sb, {50, 500, 200, 20}, HV_GREEN);
    
	draw_rectangle(m->sb, {lstate->pos.x, lstate->pos.y, 32, 32});
	draw_text(m->sb, {10, 10},tmp_format_str8(m->temp, "%d", m->sb->count), HV_GREEN);
}

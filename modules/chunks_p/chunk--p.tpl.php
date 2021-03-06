<?php

/**
 * @file
 * Paragraph chunk template.
 *
 * Available variables:
 * - $classes: The string value for the class attribute initially generated by
 *   template_preprocess().
 * - $configuration: The associative array representing the chunk's
 *   configuration.
 *   - p: A string containing markup which makes up the paragraph.
 *   - edit_in_place: A boolean indicating whether or not this chunk is being
 *     edited in place.
 * - $instance_settings: An associative array representing the chunk instance
 *   settings
 *   connected to the chunk field's instance.
 * - $chunk_type: The ChunkType object representing the chunk's type.
 * - $entity: If we are viewing an entity, this will be the entity object.
 *   Otherwise it will be FALSE.
 * - $entity_type: If we are viewing an entity, this will be the string
 *   represnting the entity's type. Otherwise it will be FALSE.
 * - $field_name: The field name for the parent chunks field.
 * - $langcode: The language code for the parent chunks field.
 * - $delta: The delta for the current chunk within the parent chunks field.
 * - $preview: A boolean indicating whether or not the chunk is being previewed.
 *
 * @see template_preprocess()
 * @see template_preprocess_chunk()
 *
 * @ingroup themeable
 */

?>

<p class="<?php print $classes; ?>" <?php if ($configuration['edit_in_place']): ?>contenteditable<?php endif; ?>><?php print filter_xss(trim(preg_replace('/(\s|<br>|<\/?div>)+/', ' ', $configuration['p'])), array('a', 'b', 'em', 'i', 'strong', 'u')); ?></p>

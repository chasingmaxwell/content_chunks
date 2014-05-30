/**
 * @file Provide some additional interactivity for the list chunk type.
 */

(function($) {

  'use strict';

  // Handle in-place editing for chunks.
  Drupal.behaviors.chunksListInPlace = {
    attach: function(context, settings) {

      $('.chunks-field', context).each(function(index, element) {
        var fieldName, langcode;

        fieldName = $(this).attr('field_name');
        langcode = $(this).attr('langcode');

        if (settings.chunks[fieldName].types.list.instance_type_settings.edit_in_place && typeof Pen === 'function') {
          // Sometimes the pen-menu doesn't like to go away. Let's force it.
          $('.pen-menu').hide();
          $(':input', context).bind('mousedown.hidePenMenu', function() {
            $('.pen-menu').hide();
          });

          // Perform actions on each list chunk with the contenteditable
          // attribute.
          $('.list-chunk[contenteditable]', element).once(function() {

            // Initialize pen.
            var editor = new Pen({
              editor: this,
                list: ['bold', 'italic', 'underline', 'createlink'],
                stay: false
            });

            // Hide regular textarea.
            $(this).prev().hide();

            // Update textarea on keyup and blur.
            $(this).bind('keyup.chunksListInPlace blur.chunksListInPlace', function(e) {
              var list_items = [];
              $(this).find('li').each(function() {
                list_items.push($(this).html().replace(/\r?\n|\r/g, ''));
              });
              list_items = list_items.join("\n");
              $(this).parent().find('textarea').val(list_items);
            });
          });

          // Trigger blur event on all contenteditable list chunks when the pen
          // toolbar is clicked so we save configuration correctly.
          $('.pen-menu').bind('click.chunksListBlur', function() {
            $('.list-chunk[contenteditable]').trigger('blur.chunksListInPlace');
          });

          // A switch in list style should cause a switch in the in-place editor as well.
          $('.list-chunk-configuration select[name$="[style]"]', element).bind('change.chunksListStyleChanged', function() {
            var chunkWrapper, delta, listConfig, configuration, inPlaceEditor, el;

            chunkWrapper = $(this).parents('.chunk-wrapper');
            delta = parseInt(chunkWrapper.attr('delta'), 10);
            listConfig = $(this).parents('.list-chunk-configuration');

            // Retrieve the configuration state from the form.
            configuration = Drupal.chunks.fields[fieldName].chunks[delta].getConfigState();

            // Add edit_in_place configuration property.
            configuration.edit_in_place = true;

            // Generate the new markup.
            inPlaceEditor = Drupal.theme.prototype.chunk__list(configuration, fieldName, langcode, delta);

            // Convert inPlaceEditor from a string to an element so we can use
            // appendChild().
            el = document.createElement('div');
            el.innerHTML = inPlaceEditor;
            inPlaceEditor = el.firstChild;

            // Insert that markup into the editor.
            listConfig.find('.list-chunk').remove();
            listConfig[0].appendChild(inPlaceEditor);
            Drupal.behaviors.chunksListInPlace.attach(chunkWrapper.parents('.field-type-chunks'), Drupal.settings);
          });
        }
      });
    }
  };

  Drupal.behaviors.chunksListCallbacks = {
    attach: function(context, settings) {

      // Implements the saveConfig callback to save list items.
      if (typeof settings.chunks.callbacks.list.restoreConfig === 'undefined') {
        settings.chunks.callbacks.list.restoreConfig = function(fieldName, langcode, delta) {

          // Only do anything if this instance of the chunk type is set to be
          // edited in-place.
          if (settings.chunks[fieldName].types.list.instance_type_settings.edit_in_place) {
            var classFieldName, listConfig, configuration, inPlaceEditor, listConfigMarkup;

            classFieldName = fieldName.replace(/_/g, '-');
            listConfig = $('#' + fieldName + '-' + delta + '-chunk .list-chunk-configuration');

            // Update configuration.
            configuration = Drupal.settings.chunks[fieldName].chunks[delta].configuration.list;
            configuration.edit_in_place = true;

            // Generate new markup.
            inPlaceEditor = Drupal.theme.prototype.chunk__list(configuration, fieldName, langcode, delta);

            // Insert that new markup into the editor.
            listConfig.find('.list-chunk').remove();
            listConfig.append(inPlaceEditor);
            Drupal.behaviors.chunksListInPlace.attach(listConfig.parents('.field-type-chunks'), Drupal.settings);
          }
        };
      }
    },
  };

  // Provide a client-side theme implementation for list chunks.
  Drupal.theme.prototype.chunk__list = function(configuration, fieldName, langcode, delta) {
    var list, output, list_item, contentEditable;

    // Break configuration.list into an array if it's value is coming from the
    // textarea element.
    if (typeof configuration.list === 'string') {
      configuration.list = configuration.list.split(/\n/);
    }

    // Add contenteditable attribute if we're editing this chunk in-place.
    if (typeof configuration.edit_in_place !== 'undefined') {
      contentEditable = configuration.edit_in_place ? ' contenteditable' : '';
    }
    else {
      contentEditable = '';
    }

    output = '<' + configuration.style + ' class="chunk list-chunk"' + contentEditable + '>';

    for (var key in configuration.list) {

      // If the format is plain text, run the text through chunkPlain
      if (Drupal.settings.chunks[fieldName].types.list.instance_type_settings.format === 'plain_text') {
        list_item = Drupal.checkPlain(configuration.list[key]);
      }
      // Otherwise don't do anything at all.
      else {
        list_item = configuration.list[key];
      }

      // Do not display empty strings.
      if (list_item === '') {
        continue;
      }

      // Add the list item to output.
      output += '<li>' + list_item + '</li>';
    }

    output += '</' + configuration.style + '>';

    return output;
  };

})(jQuery);

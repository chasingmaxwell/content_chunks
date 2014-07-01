/**
 * @file Provide some additional interactivity for the list chunk type.
 */

(function($) {

  'use strict';

  // Handle in-place editing for chunks.
  Drupal.behaviors.chunksListInPlace = {
    attach: function(context, settings) {

      $('.chunks-field', context).each(function(index, element) {
        var fieldName, langcode, fieldSettings, initializePen;

        fieldName = $(this).attr('field_name');
        langcode = $(this).attr('langcode');
        fieldSettings = settings.chunks[fieldName];
        initializePen = false;

        // Don't do anything if the Pen library isn't loaded.
        if (typeof Pen !== 'function') {
          return;
        }

        // Only initialize Pen if it's needed for a chunk instance.
        for (var instanceName in fieldSettings.instances) {
          if (fieldSettings.instances[instanceName].type === 'list' && fieldSettings.instances[instanceName].settings.edit_in_place) {
            initializePen = true;
            break;
          }
        }

        // Do not continue if there were no list chunk instances with the
        // edit_in_place setting.
        if (!initializePen) {
          return;
        }

        // Sometimes the pen-list-menu doesn't like to go away. Let's force it.
        $('.pen-list-menu').hide();
        $(':input', context).bind('mousedown.hidePenMenu' + instanceName, function() {
          $('.pen-list-menu').hide();
        });

        // Perform actions on each list chunk with the contenteditable
        // attribute.
        $('.list-chunk[contenteditable]', element).once(function() {

          // Initialize pen.
          var editor = new Pen({
            class: 'pen-list',
            editor: this,
            list: ['bold', 'italic', 'underline', 'createlink'],
            stay: false
          });

          // Hide regular textarea.
          $(this).prev().hide();

          // Update textarea on keyup, blur, paste, and cut events.
          $(this).bind('keyup.chunksListInPlace blur.chunksListInPlace paste.chunksListInPlace cut.chunksListInPlace', function(e) {
            var list_items = [],
            editableElement = this;

            // Each time the configuration could have changed, copy the data
            // back into the form item so it gets saved when the user submits
            // the form. setTimeout is necessary because some events fire
            // before the markup has changed.
            setTimeout(function() {
              $(editableElement).find('li').each(function() {
                list_items.push($(this).html().replace(/\r?\n|\r/g, ''));
              });
              list_items = list_items.join("\n");
              $(editableElement).parent().find('textarea').val(list_items);
            }, 0);
          });

          // A switch in list style should cause a switch in the in-place editor as well.
          $('select[name$="[style]"]', $(this).parent()).bind('change.chunksListStyleChanged', function() {
            var chunkWrapper, delta, listConfig, configuration, inPlaceEditor, el;

            chunkWrapper = $(this).parents('.chunk-wrapper');
            delta = parseInt(chunkWrapper.attr('delta'), 10);
            listConfig = $(this).parents('.list-chunk-type-configuration');

            // Retrieve the configuration state from the form.
            configuration = Drupal.chunks.fields[fieldName].chunks[delta].getConfigState();

            // Add edit_in_place configuration property.
            configuration.edit_in_place = true;

            // Make sure we have a placeholder if the list is empty.
            if (configuration.list.length < 1) {
              configuration.list = 'Enter list items here...';
            }

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
        });

        // Trigger blur event on all contenteditable list chunks when the pen
        // toolbar is clicked so we save configuration correctly.
        $('.pen-list-menu').bind('click.chunksListBlur', function() {
          $('.list-chunk[contenteditable]').trigger('blur.chunksListInPlace');
        });
      });
    }
  };

  Drupal.behaviors.chunksListCallbacks = {
    attach: function(context, settings) {

      // No need to do anything if we've already created the callback.
      if (typeof Drupal.settings.chunks.callbacks.restoreConfig.list !== 'undefined') {
        return;
      }

      // Implements the restoreConfig callback to restore list configuration.
      Drupal.settings.chunks.callbacks.restoreConfig.list = function(fieldName, langcode, delta) {
        var chunkInstance;

        // Retrieve the chunk instance.
        chunkInstance = Drupal.chunks.fields[fieldName].chunks[delta].chunkInstance;

        // Only do anything if this instance of the chunk is set to be edited
        // in-place.
        if (Drupal.settings.chunks[fieldName].instances[chunkInstance].settings.edit_in_place) {
          var classFieldName, listConfig, configState, configuration, inPlaceEditor;

          classFieldName = fieldName.replace(/_/g, '-');
          listConfig = $('#' + fieldName + '-' + delta + '-chunk .list-chunk-type-configuration');

          // Make a copy of the configuration and add the edit_in_place
          // property so we can render a contenteditable list chunk
          // without changing the configuration settings for the chunk.
          configState = Drupal.settings.chunks[fieldName].chunks[delta].configuration[chunkInstance];
          configuration = {};
          for (var prop in configState) {
            configuration[prop] = configState[prop];
          }
          configuration.edit_in_place = true;

          // Generate new markup.
          inPlaceEditor = Drupal.theme.prototype.chunk__list(configuration, fieldName, langcode, delta);

          // Insert that new markup into the editor.
          listConfig.find('.list-chunk').remove();
          listConfig.append(inPlaceEditor);
          Drupal.behaviors.chunksListInPlace.attach(listConfig.parents('.field-type-chunks'), Drupal.settings);
        }
      };
    },
  };

  // Provide a client-side theme implementation for list chunks.
  Drupal.theme.prototype.chunk__list = function(configuration, fieldName, langcode, delta) {
    var list, output, list_item, contentEditable, chunkInstance;

    // Retrieve the chunk instance.
    chunkInstance = Drupal.chunks.fields[fieldName].chunks[delta].chunkInstance;

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
      if (Drupal.settings.chunks[fieldName].instances[chunkInstance].settings.format === 'plain_text') {
        list_item = Drupal.checkPlain(configuration.list[key]);
      }
      // Otherwise don't do anything at all.
      else {
        list_item = configuration.list[key];
      }

      // Do not display empty strings.
      // '<p><br></p>' is the merkup output by Pen when a line is empty.
      if (list_item === '' || list_item == '<p><br></p>') {
        continue;
      }

      // Add the list item to output.
      output += '<li>' + list_item + '</li>';
    }

    output += '</' + configuration.style + '>';

    return output;
  };

})(jQuery);
